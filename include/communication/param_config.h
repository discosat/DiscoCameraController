#ifndef PARAM_CONFIG_H
#define PARAM_CONFIG_H

#include <param/param.h>
extern param_t capture_param;
extern param_t camera_id_param;
extern param_t camera_type_param;
extern param_t camera_state_param;
extern param_t exposure_param;
extern param_t iso_param;
extern param_t num_images_param;
extern param_t interval_param;
extern param_t obid_param;
extern param_t pipeline_id_param;
extern param_t error_log;

#define CAMERA_ID_MAX_LENGTH 128

#define PARAMID_CAPTURE_PARAM       1
#define PARAMID_CAMERA_ID_PARAM     2 
#define PARAMID_CAMERA_TYPE_PARAM   3 
#define PARAMID_CAMERA_STATE_PARAM  4
#define PARAMID_EXPOSURE_PARAM      5
#define PARAMID_ISO_PARAM           6
#define PARAMID_NUM_IMAGES_PARAM    7
#define PARAMID_INTERVAL_PARAM      8
#define PARAMID_OBID_PARAM          9
#define PARAMID_PIPELINE_ID_PARAM   10
#define PARAMID_ERROR_LOG           11

#define PARAM_MAX_SIZE 512

#endif /* PARAM_CONFIG_H */
