#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

#include <vector>
#include "camera_controller.hpp"
#include "common.hpp"

#define TEST_WIDTH 2592
#define TEST_HEIGHT 1944
#define TEST_BPP 12
#define TEST_TEST_CHANNELS 1
#define TEST_FILE_LOCATION "/home/ivar/Dicso2CameraControl/test_code/image_1712754488_1_12bit.bayerRG"

class TestController: public CameraController {
    public:
        TestController();
        ~TestController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
};

#endif