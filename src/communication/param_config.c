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

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_CAPTURE_PARAM,
    capture_param,
    PARAM_TYPE_STRING,
    PARAM_MAX_SIZE,
    1,
    PM_REMOTE,
    capture_param_callback,
    NULL,
    config,
    VMEM_CONF_CAPTURE_PARAM,
    "Set instructions on image capture, e.g. CAMERA=XXX;EXPOSURE=XXX;ISO=XXX;"
);
