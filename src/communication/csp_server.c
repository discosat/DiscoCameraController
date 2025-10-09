#include "csp_server.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <csp/csp.h>
#include <csp/interfaces/csp_if_zmqhub.h>
#include <param/param_server.h>
#include <vmem/vmem_file.h>
#include <vmem/vmem_server.h>

#include "param_config.h"
#include "vmem_config.h"
#include <csp/drivers/can_socketcan.h>
#include <csp/drivers/usart.h>
#include <csp/interfaces/csp_if_can.h>
#include <csp/interfaces/csp_if_kiss.h>
#include <errno.h> // for system errors
#include <errors.hpp>
#include <sys/types.h>
#include <string.h> // for string operations

/*
    Error codes for "errno" can be found here:
    https://gist.github.com/greggyNapalm/2413028
*/

// shared resources and mutexes
char camera_id[CAMERA_ID_MAX_LENGTH];
char current_active_camera_id[CAMERA_ID_MAX_LENGTH] = "";
uint8_t capture;
uint8_t camera_type;
uint32_t exposure;
double iso;
uint32_t num_images;
uint32_t interval;
uint32_t obid;
uint32_t pipeline_id;
pthread_mutex_t mutex;
pthread_cond_t cond;

// For catching ctrl-c
static volatile int _RUNNING = 1;
void intHandler(int _) {
  _RUNNING = 0;
  pthread_cond_signal(&cond);
}

void *vmem_server_task(void *param) {
  vmem_server_loop(param);
  return NULL;
}

void *router_task(void *param) {
  while (1) {
    csp_route_work();
  }
  return NULL;
}

// Helper function to control GPIO pins based on camera model name
void set_camera_gpio(const char* camera_model, int state) {
  char gpio_cmd[256];

  if (state == 0) {
    // Turn off all cameras
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=0;gpioset gpiochip2 0=0");
    printf("Turning off all cameras\n");
  } else if (strcmp(camera_model, CAMERA_1_MODEL) == 0) {
    // Camera 1 (1800 U-507c): pin1=1, pin0=0
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=1;gpioset gpiochip2 0=0");
    printf("Switching to %s\n", camera_model);
  } else if (strcmp(camera_model, CAMERA_2_MODEL) == 0) {
    // Camera 2 (1800 U-811c): pin1=0, pin0=1
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=0;gpioset gpiochip2 0=1");
    printf("Switching to %s\n", camera_model);
  } else if (strcmp(camera_model, CAMERA_3_MODEL) == 0) {
    // Camera 3 (Boson): pin1=1, pin0=1
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=1;gpioset gpiochip2 0=1");
    printf("Switching to %s\n", camera_model);
  } else {
    printf("Unknown camera model: %s\n", camera_model);
    return;
  }

  // Execute the GPIO command
  int result = system(gpio_cmd);
  if (result == 0) {
    if (state == 0) {
      printf("GPIO pins configured: All cameras OFF\n");
    } else {
      printf("GPIO pins configured: %s is now active\n", camera_model);
    }
  } else {
    printf("Failed to configure GPIO pins. Command result: %d\n", result);
    if (result == 32512) {
      printf("Note: gpioset command not found. Install with: sudo apt-get install gpiod\n");
    }
  }
}

void camera_id_param_callback() {
  char new_camera_id[CAMERA_ID_MAX_LENGTH];
  param_get_string(&camera_id_param, new_camera_id, CAMERA_ID_MAX_LENGTH);

  uint8_t camera_state = param_get_uint8(&camera_state_param);

  printf("Camera ID changed to: %s\n", new_camera_id);

  // If there's a currently active camera and it's different from the new one
  if (strlen(current_active_camera_id) > 0 && strcmp(current_active_camera_id, new_camera_id) != 0) {
    printf("Switching from camera %s to %s\n", current_active_camera_id, new_camera_id);

    // Turn off all cameras when switching
    set_camera_gpio(NULL, 0);

    // Set camera_state_param to 0 to indicate camera is off
    param_set_uint8(&camera_state_param, 0);
  }

  // Update the current active camera ID
  strcpy(current_active_camera_id, new_camera_id);

  // If camera_state is 1, turn on the new camera
  if (camera_state == 1) {
    set_camera_gpio(new_camera_id, 1);
    // Update camera_state_param to 1 to reflect that the new camera is on
    param_set_uint8(&camera_state_param, 1);
  }
}

void camera_state_param_callback() {
  uint8_t camera_state = param_get_uint8(&camera_state_param);
  char camera_id_str[CAMERA_ID_MAX_LENGTH];
  param_get_string(&camera_id_param, camera_id_str, CAMERA_ID_MAX_LENGTH);

  printf("Camera state change: %s (camera_id: %s)\n",
         camera_state ? "ON" : "OFF", camera_id_str);

  if (camera_state == 0) {
    // Turn off all cameras
    set_camera_gpio(NULL, 0);
    // Clear the current active camera
    current_active_camera_id[0] = '\0';
  } else if (camera_state == 1) {
    // Turn on the camera specified by camera_id_param
    set_camera_gpio(camera_id_str, 1);
    // Update the current active camera
    strcpy(current_active_camera_id, camera_id_str);
  } else {
    printf("Invalid camera state: %u (should be 0 or 1)\n", camera_state);
  }
}

void capture_param_callback() {

  uint8_t param_value = param_get_uint8(&capture_param);

  if (!param_value)
    return;
  capture = param_get_uint8(&capture_param);

  pthread_mutex_lock(&mutex);

  // Get the camera_id parameter (camera model name like "1800 U-507c")
  param_get_string(&camera_id_param, camera_id, CAMERA_ID_MAX_LENGTH);

  printf("Capture requested for camera: %s\n", camera_id);

  camera_type = param_get_uint8(&camera_type_param);
  exposure = param_get_uint32(&exposure_param);
  iso = param_get_double(&iso_param);
  num_images = param_get_uint32(&num_images_param);
  interval = param_get_uint32(&interval_param);
  obid = param_get_uint32(&obid_param);
  pipeline_id = param_get_uint32(&pipeline_id_param);

  /*
  strcpy(camera_id, "1800 U-500c");  // Use actual camera
  camera_type = 0;  // VMB camera type
  exposure = 50000;  // Increased from 5ms to 50ms for much brighter images
  iso = 4.0;         // Increased from 1.0 to 4.0 for 4x gain boost
  num_images = 1;
  interval = 0;
  obid = 0;
  pipeline_id = 0;
  */

  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

static void csp_init_fun(void) {
  csp_conf.hostname = "Camera";
  csp_conf.model = "DISCO-II";
  csp_conf.revision = "1";
  csp_conf.version = 2;
  csp_conf.dedup = CSP_DEDUP_OFF;

  csp_init();

  csp_bind_callback(csp_service_handler, CSP_ANY);
  csp_bind_callback(param_serve, PARAM_PORT_SERVER);

  static pthread_t vmem_server_handle;
  pthread_create(&vmem_server_handle, NULL, &vmem_server_task, NULL);

  static pthread_t router_handle;
  pthread_create(&router_handle, NULL, &router_task, NULL);

  // static pthread_t onehz_handle;
  // pthread_create(&onehz_handle, NULL, &onehz_task, NULL);
}

/// @brief Initialize communication interfaces: ZMQ, CAN and KISS
/// @param interfaceConfig configuration parameters
static void iface_init(CSPInterface *interfaceConfig) {
  int error = CSP_ERR_NONE;
  csp_iface_t *default_iface = NULL;

  switch (interfaceConfig->Interface) {
  case ZMQ:
    error = csp_zmqhub_init_filter2(
        "zmq", interfaceConfig->Device, interfaceConfig->Node, 8, true,
        &default_iface, NULL, CSP_ZMQPROXY_SUBSCRIBE_PORT,
        CSP_ZMQPROXY_PUBLISH_PORT);
    csp_print("Value of errno: %d\n", errno);
    default_iface->name = "zmq";
    break;
  case CAN:
    error = csp_can_socketcan_open_and_add_interface(
        interfaceConfig->Device, "CAN", interfaceConfig->Node, 0, 0,
        &default_iface);
    csp_print("Value of errno: %d\n", errno);
    default_iface->name = "CAN";
    break;
  case KISS:
    csp_usart_conf_t conf = {
        .device = interfaceConfig->Device,
        .baudrate = 115200, /* supported on all platforms */
        .databits = 8,
        .stopbits = 1,
        .paritysetting = 0,
    };
    error = csp_usart_open_and_add_kiss_interface(
        &conf, CSP_IF_KISS_DEFAULT_NAME, &default_iface);
    default_iface->addr = interfaceConfig->Node;
    csp_print("Value of errno: %d\n", errno);
    default_iface->name = "kiss";
    break;
  }

  if (error != CSP_ERR_NONE) {
    csp_print("failed to add interface [%s], error: %d\n",
              interfaceConfig->Device, error);
    exit(1);
  } else {
    csp_print("Initialized interface:\n\t - Device: [%s]\n\t - Node: %i\n\t - "
              "Interface mode: %s\n",
              interfaceConfig->Device, interfaceConfig->Node,
              default_iface->name);
  }

  default_iface->is_default = 1;
  default_iface->addr = interfaceConfig->Node;
  default_iface->netmask = 8;
  csp_rtable_set(0, 0, default_iface, CSP_NO_VIA_ADDRESS);
  csp_iflist_add(default_iface);
}

void server_start(CSPInterface *interfaceConfig, CallbackFunc callback,
                  void *obj) {
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  void serial_init(void);
  serial_init();

  // Parameter storage
  vmem_file_init(&vmem_config);

  // Interfaces
  iface_init(interfaceConfig);

  // Initialize CSP
  csp_init_fun();
  param_set_string(&capture_param, "", PARAM_MAX_SIZE);

  signal(SIGINT, intHandler);
  while (_RUNNING) {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    u_int16_t error = 0;

    if (capture > 0 && _RUNNING) {
      csp_print("Using camera_id: %s\n", camera_id);
      callback(camera_id, camera_type, exposure, iso, num_images, interval,
               obid, pipeline_id, obj, &error);
      param_set_uint8(&capture_param, 0); // Reset to zero.

      // Turn off the camera after capture is complete
      param_set_uint8(&camera_state_param, 0);
    }
    pthread_mutex_unlock(&mutex);
    param_set_uint16(&error_log, error);
  }
}
