#ifndef VMEM_CONFIG_H
#define VMEM_CONFIG_H

#include <vmem/vmem.h>
extern vmem_t vmem_config;

#define VMEM_CONF_CAPTURE_PARAM 0x64 // 0x000       // 1 byte
#define VMEM_CONF_CAMERA_ID_PARAM 0xc8 // 0x100     // 256 bytes
#define VMEM_CONF_CAMERA_TYPE_PARAM 0x320  // 0x008   // 1 byte
#define VMEM_CONF_CAMERA_STATE_PARAM 0x321 // 0x009  // 1 byte
#define VMEM_CONF_CAMERA_TEMPERATURE_PARAM 0x322 // 0x00A  // 8 bytes
#define VMEM_CONF_EXPOSURE_PARAM 0x384  // 0x010      // 4 bytes
#define VMEM_CONF_ISO_PARAM 0x3e8  // 0x030           // 8 bytes
#define VMEM_CONF_NUM_IMAGES_PARAM 0x44c  // 0x070    // 4 bytes
#define VMEM_CONF_INTERVAL_PARAM 0x4b0  // 0x090      // 4 bytes
#define VMEM_CONF_OBID_PARAM 0x514  // 0x0b0          // 4 bytes
#define VMEM_CONF_PIPELINE_ID_PARAM 0x578  // 0x0d0   // 4 bytes
#define VMEM_CONF_ERROR_PARAM 0x5dc  // 0x0f0         // 2 bytes
#define VMEM_CONF_CAMERA_TEMP_READ_PARAM 0x5de  // 0x0f2   // 1 byte (trigger)

#endif /* VMEM_CONFIG_H */
