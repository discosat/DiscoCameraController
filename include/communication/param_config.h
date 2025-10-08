#ifndef PARAM_CONFIG_H
#define PARAM_CONFIG_H

#include <param/param.h>
extern param_t capture_param;
extern param_t camera_id_param;
extern param_t camera_type_param;
extern param_t camera_state_param;
extern param_t camera_temperature_param;
extern param_t exposure_param;
extern param_t iso_param;
extern param_t num_images_param;
extern param_t interval_param;
extern param_t obid_param;
extern param_t pipeline_id_param;
extern param_t error_log;

#define CAMERA_ID_MAX_LENGTH 128

// Camera configuration mapping
// Production camera model definitions
#define CAMERA_1_MODEL "1800 U-507c"  // cam1: Optical camera 1 (5MP)
#define CAMERA_2_MODEL "1800 U-811c"  // cam2: Optical camera 2 (12MP)
#define CAMERA_3_MODEL "IR_Camera"     // cam3: IR camera (model TBD)
#define CAMERA_4_MODEL "1800 U-500c"  // cam4: Test camera (5MP)

// GPIO mapping for camera switching
#define CAMERA_1_GPIO_NUM 1  // GPIO: pin1=0, pin0=1
#define CAMERA_2_GPIO_NUM 2  // GPIO: pin1=1, pin0=0
#define CAMERA_3_GPIO_NUM 3  // GPIO: pin1=1, pin0=1
#define CAMERA_4_GPIO_NUM 1  // Test camera shares cam1 GPIO

#define PARAMID_CAPTURE_PARAM       1
#define PARAMID_CAMERA_ID_PARAM     2 
#define PARAMID_CAMERA_TYPE_PARAM   3 
#define PARAMID_CAMERA_STATE_PARAM  4
#define PARAMID_CAMERA_TEMPERATURE_PARAM 5
#define PARAMID_EXPOSURE_PARAM      6
#define PARAMID_ISO_PARAM           7
#define PARAMID_NUM_IMAGES_PARAM    8
#define PARAMID_INTERVAL_PARAM      9
#define PARAMID_OBID_PARAM          10
#define PARAMID_PIPELINE_ID_PARAM   11
#define PARAMID_ERROR_LOG           12

#define PARAM_MAX_SIZE 512

// Forward declarations for callback functions
#ifdef __cplusplus
extern "C" {
#endif
void capture_param_callback();
void camera_id_param_callback();
void camera_state_param_callback();
void camera_temperature_callback();
#ifdef __cplusplus
}
#endif

#endif /* PARAM_CONFIG_H */
