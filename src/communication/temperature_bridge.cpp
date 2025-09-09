#include "vimba_controller.hpp"
#include "param_config.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>
#include <iostream>

// Global VimbaController instance for temperature updates
static VimbaController* g_vimba_controller = nullptr;
static std::thread* g_temperature_thread = nullptr;
static std::atomic<bool> g_temperature_thread_running(false);
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

void temperature_update_loop() {
    while (g_temperature_thread_running.load()) {
        if (g_vimba_controller) {
            // Update temperature parameter
            bool success = g_vimba_controller->updateTemperatureParameter();
            
            if (success) {
                // Check temperature threshold
                double current_temp = param_get_double(&camera_temperature_param);
                
                if (current_temp >= TEMPERATURE_THRESHOLD) {
                    emergency_shutdown_camera();
                    // Continue monitoring even after shutdown
                }
            }
        }
        // Update temperature every 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

extern "C" {
    void init_temperature_controller() {
        try {
            g_vimba_controller = new VimbaController();
            g_temperature_thread_running.store(true);
            g_temperature_thread = new std::thread(temperature_update_loop);
            printf("Temperature controller initialized with 1-second updates and 65°C shutdown protection\n");
        } catch (const std::exception& e) {
            printf("Failed to initialize VimbaController for temperature updates: %s\n", e.what());
            g_vimba_controller = nullptr;
        }
    }
    
    void cleanup_temperature_controller() {
        if (g_temperature_thread) {
            g_temperature_thread_running.store(false);
            g_temperature_thread->join();
            delete g_temperature_thread;
            g_temperature_thread = nullptr;
        }
        if (g_vimba_controller) {
            delete g_vimba_controller;
            g_vimba_controller = nullptr;
        }
        printf("Temperature controller cleaned up\n");
    }
    
    void update_camera_temperature() {
        if (g_vimba_controller) {
            bool success = g_vimba_controller->updateTemperatureParameter();
            if (!success) {
                printf("Failed to update camera temperature\n");
            }
        } else {
            printf("VimbaController not initialized for temperature updates\n");
        }
    }
}