# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

**Build the project:**
```bash
./build.sh
```
This removes the build directory, runs meson setup, and builds with ninja.

**Run the application:**
```bash
./run.sh
```
Runs with default parameters: ZMQ interface, node 163, localhost device.

**Manual execution:**
```bash
./build/Disco2CameraControl -i INTERFACE -d DEVICE -n NODE -p PORT
```
- INTERFACE: zmq, can, or kiss (default: zmq)
- DEVICE: connection device (default: localhost) 
- NODE: CSP node address (default: 2)
- PORT: server listening port (default: 10)

## Architecture Overview

### Core Components

**CaptureController** (`src/camera_control/capture_controller.cpp`)
- Main orchestrator that handles capture requests
- Creates appropriate camera controller instances based on camera type
- Manages the entire capture workflow from parameter validation to image delivery

**Camera Controllers** (Abstract factory pattern)
- `VimbaController`: Handles Allied Vision cameras via VimbaX SDK
- `IRController`: Handles infrared cameras  
- `TestController`: Provides mock camera functionality for testing
- All inherit from `CameraController` base class

**MessageQueue** (`src/message_queue/message_queue.cpp`)
- Manages System V shared memory and message queues
- Handles image data transfer to the image processing pipeline (DIPP)
- Creates shared memory segments and sends notification messages

**CSP Server** (`src/communication/csp_server.c`)
- Provides CSP (Cubesat Space Protocol) communication interface
- Exposes parameters via libparam for remote control
- Handles different transport interfaces (ZMQ, CAN, KISS)

### Data Flow

1. **Parameter Setting**: Ground station sets capture parameters via CSP
2. **Capture Trigger**: `capture_param` set to 1 triggers image acquisition
3. **Controller Selection**: CaptureController creates appropriate camera controller
4. **Image Capture**: Camera controller captures raw BayerRG images (12-bit)
5. **Data Packaging**: Images packaged with metadata into batch structure
6. **Memory Management**: Image data stored in System V shared memory
7. **Pipeline Notification**: Message queue notifies DIPP of ready images

### Key Data Structures

**ImageBatch**: System V message queue structure containing image metadata and shared memory key
**Image**: Individual image structure with dimensions, pixel data, and timestamps  
**CaptureMessage**: Internal capture instruction format with all parameters

### Build System

- **Meson** build system with cross-compilation support
- Automatically detects ARM64 vs x86_64 and links appropriate VimbaX libraries
- Dependencies: CSP, libparam, OpenCV, Protobuf, VimbaX SDK

### Camera Parameters (CSP exposed)

- `camera_id_param`: Camera model string (e.g., "1800 U-500c")
- `camera_type_param`: 0=VMB, 1=IR, 2=TEST
- `exposure_param`: Exposure in microseconds
- `iso_param`: Gain/ISO value
- `num_images_param`: Number of images in burst
- `interval_param`: Delay between images (microseconds)
- `obid_param`: Unique batch identifier
- `pipeline_id_param`: Target processing pipeline ID
- `capture_param`: Set to 1 to trigger capture
- `error_log`: Latest error code (read-only)

### Error Handling

Error codes are defined in `include/utils/errors.hpp`:
- 100s: Parameter parsing errors
- 200s: Camera/capture errors  
- 300s: Message queue/shared memory errors

## Development Notes

### Prerequisites
```bash
sudo apt-get install libzmq3-dev libbsd
```

**Protobuf 3.19** is required - clone from the 3.19.x branch and build manually.

### Testing
- Use `camera_type_param = 2` (TEST) for testing without hardware
- Test executable `receiver` in build directory can receive image messages
- Debug mode available with `-D` flag

### VimbaX Integration
- Cross-compilation support for ARM64 vs Linux x86_64
- VimbaX SDK libraries automatically selected based on build target
- Raw BayerRG format captured at 12-bit depth for maximum processing flexibility