#ifndef TEMPERATURE_BRIDGE_H
#define TEMPERATURE_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

void init_temperature_controller();
void cleanup_temperature_controller();
void update_camera_temperature();
void camera_temperature_callback();

#ifdef __cplusplus
}
#endif

#endif // TEMPERATURE_BRIDGE_H