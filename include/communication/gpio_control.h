#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

// GPIO control function for camera power management
// camera_model: Camera model name (e.g., "1800 U-507c"), or NULL to turn off all
// state: 0 = off, 1 = on
void set_camera_gpio(const char* camera_model, int state);

#ifdef __cplusplus
}
#endif

#endif // GPIO_CONTROL_H