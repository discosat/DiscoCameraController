#ifndef PARAM_CONFIG_H
#define PARAM_CONFIG_H

#include <param/param.h>
extern param_t capture_param;
extern param_t error_log;

#define PARAMID_CAPTURE_PARAM       1
#define PARAMID_CAMERA_ID_PARAM     2 
#define PARAMID_CAMERA_TYPE_PARAM   3 
#define PARAMID_EXPOSURE_PARAM      4 
#define PARAMID_ISO_PARAM           5 
#define PARAMID_NUM_IMAGES_PARAM    6 
#define PARAMID_INTERVAL_PARAM      7 
#define PARAMID_OBID_PARAM          8 
#define PARAMID_ERROR_LOG           9

#define PARAM_MAX_SIZE 312 // Error log is 128 bytes

#endif /* PARAM_CONFIG_H */
