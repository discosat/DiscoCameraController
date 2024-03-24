#include <vmem/vmem_file.h>

// TODO: Problematic for Cortex-M7
// Define file to store persistent params
VMEM_DEFINE_FILE(config, "config", "config.vmem", 5000);
