#include "vimba_controller.hpp"
#include <cstdlib>
#include <iostream>
#include <atomic>

// this should fix linking errors?
extern "C" {
    #include "param_config.h"
    #include <param/param.h>
}

// Global shutdown tracking
static std::atomic<bool> g_temperature_shutdown_triggered(false);

const double TEMPERATURE_THRESHOLD = 65.0; // degrees Celsius

void emergency_shutdown_camera() {
    if (g_temperature_shutdown_triggered.exchange(true)) {
        return; // Already triggered, avoid multiple shutdowns
    }
    
    printf("CRITICAL: Camera temperature exceeded %.1f°C - Emergency shutdown initiated!\n", TEMPERATURE_THRESHOLD);
    
    // Turn off all cameras using GPIO (same as camera_state = 0)
    char gpio_cmd[256];
    snprintf(gpio_cmd, sizeof(gpio_cmd), "gpioset gpiochip2 1=0 0=0");
    printf("Emergency GPIO shutdown: %s\n", gpio_cmd);
    
    int result = system(gpio_cmd);
    if (result == 0) {
        printf("Emergency GPIO shutdown successful - All cameras turned OFF\n");
    } else {
        printf("ERROR: Emergency GPIO shutdown failed! Command result: %d\n", result);
    }
    
    // Also set the camera_state_param to 0 to reflect the shutdown
    param_set_uint8(&camera_state_param, 0);
}

// On-demand temperature reading with emergency shutdown check
bool read_camera_temperature_on_demand() {
    VimbaController temp_controller;
    
    try {
        bool success = temp_controller.updateTemperatureParameter();
        
        if (success) {
            // Check temperature threshold
            double current_temp = param_get_double(&camera_temperature_param);
            
            if (current_temp >= TEMPERATURE_THRESHOLD) {
                emergency_shutdown_camera();
            }
            return true;
        }
    } catch (const std::exception& e) {
        printf("Failed to read camera temperature: %s\n", e.what());
    }
    return false;
}

extern "C" {
    void init_temperature_controller() {
        printf("Temperature controller initialized (on-demand mode) with 65°C shutdown protection\n");
    }
    
    void cleanup_temperature_controller() {
        printf("Temperature controller cleaned up\n");
    }
    
    void update_camera_temperature() {
        if (!read_camera_temperature_on_demand()) {
            printf("Failed to update camera temperature\n");
        }
    }
    
    void camera_temperature_callback() {
        // This callback is triggered when someone accesses the temperature parameter
        // We update the temperature on-demand rather than continuously
        read_camera_temperature_on_demand();
    }
}