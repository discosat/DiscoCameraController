#include "gpio_control.h"
#include "logger.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Camera model definitions (should match param_config.h)
#define CAMERA_1_MODEL "1800 U-507c"
#define CAMERA_2_MODEL "1800 U-811c"
#define CAMERA_3_MODEL "Boson"
#define CAMERA_4_MODEL "1800 U-500c"  // Test camera

// Helper function to control GPIO pins based on camera model name
// Note: All gpioset commands use shorthand syntax where multiple pin settings are space-separated
// e.g., "gpioset gpiochip2 1=0 0=0" is equivalent to "gpioset gpiochip2 1=0;gpioset gpiochip2 0=0"
void set_camera_gpio(const char* camera_model, int state) {
  char gpio_cmd[256];

  if (state == 0) {
    // Turn off all cameras
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=0 0=0");
    log_gpio("Powering OFF all cameras");
  } else if (strcmp(camera_model, CAMERA_1_MODEL) == 0) {
    // Camera 1 (1800 U-507c): pin1=1, pin0=0
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=1 0=0");
    char msg[128];
    snprintf(msg, sizeof(msg), "Powering ON: %s", camera_model);
    log_gpio(msg);
  } else if (strcmp(camera_model, CAMERA_2_MODEL) == 0) {
    // Camera 2 (1800 U-811c): pin1=0, pin0=1
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=0 0=1");
    char msg[128];
    snprintf(msg, sizeof(msg), "Powering ON: %s", camera_model);
    log_gpio(msg);
  } else if (strcmp(camera_model, CAMERA_3_MODEL) == 0) {
    // Camera 3 (Boson): pin1=1, pin0=1
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=1 0=1");
    char msg[128];
    snprintf(msg, sizeof(msg), "Powering ON: %s", camera_model);
    log_gpio(msg);
  } else if (strcmp(camera_model, CAMERA_4_MODEL) == 0) {
    // Camera 4 (1800 U-500c): Test camera - no GPIO control needed
    char msg[128];
    snprintf(msg, sizeof(msg), "Test camera %s - no GPIO control (USB powered)", camera_model);
    log_info(msg);
    return;  // Exit without trying to set GPIO pins
  } else {
    char msg[128];
    snprintf(msg, sizeof(msg), "Unknown camera model: %s", camera_model);
    log_warning(msg);
    return;
  }

  // Execute the GPIO command
  int result = system(gpio_cmd);
  if (result == 0) {
    if (state == 0) {
      log_success("GPIO configured: All cameras OFF");
    } else {
      char msg[128];
      snprintf(msg, sizeof(msg), "GPIO configured: %s is active", camera_model);
      log_success(msg);
    }
  } else {
    char msg[256];
    snprintf(msg, sizeof(msg), "Failed to configure GPIO pins (error: %d)", result);
    log_error(msg);
    if (result == 32512) {
      log_error("gpioset command not found! Install with: sudo apt-get install gpiod");
    }
  }
}