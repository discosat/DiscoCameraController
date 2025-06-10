#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

#include <vector>
#include "camera_controller.hpp"
#include "common.hpp"

#define TEST_WIDTH 2592
#define TEST_HEIGHT 1944
#define TEST_BPP 12
#define TEST_TEST_CHANNELS 1
#define TEST_FILE_LOCATION "/home/mseo/testing_folder/DiscoCameraController/test_img_1.jpg"

class TestController: public CameraController {
    public:
        TestController();
        ~TestController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
};

#endif