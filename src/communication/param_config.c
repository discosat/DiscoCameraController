#include "param_config.h"
#include "vmem_config.h"
//#include "measure.h"
#include "csp_server.h"

/*
PARAM_DEFINE_STATIC_VMEM(
    id,           // unique parameter id, from param_config.h
    name,         // param_t variable name, from param_config.h
    type,         // Available types in enum param_type_e in param.h
    array_count,  // Number of elements/(bytes?) in the array. -1 for single values
    array_step,   // ??maybe element size in bytes. 0 for single values, 1 for arrays
    flags,        // See param.h; PM_SYSCONF for system/network config, PM_CONF for user config, PM_READONLY for read-only
    callback,     // Callback function for when the value is set
    unit,         // ?? Possibly the unit of the value, e.g. "m/s"
    vmem_name,    // Name of the vmem, without "vmem_" prefix Defined in vmem_config.h. Initialized in main.c
    vmem_address, // Offset in the vmem, from vmem_config.h
    docstr        // Documentation string (for param info)
)
*/

// Capture

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_CAPTURE_PARAM,
    capture_param,
    PARAM_TYPE_UINT8,
    -1,
    0,
    PM_CONF,
    capture_param_callback,
    NULL,
    config,
    VMEM_CONF_CAPTURE_PARAM,
    "Turn on capture by setting a value other than 0."
);

// Camera ID

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_CAMERA_ID_PARAM,
    camera_id_param,
    PARAM_TYPE_STRING,
    CAMERA_ID_MAX_LENGTH,
    1,
    PM_CONF, 
    NULL,
    NULL,
    config,
    VMEM_CONF_CAMERA_ID_PARAM,
    "The id of the camera to capture with"
);

// Camera type

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_CAMERA_TYPE_PARAM,
    camera_type_param,
    PARAM_TYPE_UINT8,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_CAMERA_TYPE_PARAM,
    "The camera type to capture with, by default VMB"
);

// Exposure

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_EXPOSURE_PARAM,
    exposure_param,
    PARAM_TYPE_UINT32,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_EXPOSURE_PARAM,
    "Exposure in microseconds. If value is not set, then exposure is estimated."
);

// ISO

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_ISO_PARAM,
    iso_param,
    PARAM_TYPE_DOUBLE,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_ISO_PARAM,
    "ISO or gain. By default 1."
);

// Number of images

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_NUM_IMAGES_PARAM,
    num_images_param,
    PARAM_TYPE_UINT32,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_NUM_IMAGES_PARAM,
    "Number of images to capture. By default 1."
);

// Interval

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_INTERVAL_PARAM,
    interval_param,
    PARAM_TYPE_UINT32,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_INTERVAL_PARAM,
    "Delay between images in microseconds (not including exposure). By default 0."
);

// OBID

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_OBID_PARAM,
    obid_param,
    PARAM_TYPE_UINT32,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_OBID_PARAM,
    "Observation identifier. No default. Is required."
);

// Pipeline ID

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_PIPELINE_ID_PARAM,
    pipeline_id_param,
    PARAM_TYPE_UINT32,
    -1,
    0,
    PM_CONF,
    NULL,
    NULL,
    config,
    VMEM_CONF_PIPELINE_ID_PARAM,
    "Pipeline identifier"
);

// Error log

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_ERROR_LOG,
    error_log,
    PARAM_TYPE_UINT16,
    -1,
    0,
    PM_READONLY,
    NULL,
    NULL,
    config,
    VMEM_CONF_ERROR_PARAM,
    "Latest error code"
);