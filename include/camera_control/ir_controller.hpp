#ifndef IR_CONTROLLER_H
#define IR_CONTROLLER_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "camera_controller.hpp"

#define WIDTH 640
#define HEIGHT 512
#define BUFFERSIZE WIDTH * HEIGHT * sizeof(uint16_t)
#define IR_BPP 16 // Bits per pixel in the raw output of Y16

class IRController: public CameraController {
    private:
        bool openCamera(cv::VideoCapture &cap,std::string port_name);
        void closeCamera(cv::VideoCapture &cap);
        u_char* captureFrame(cv::VideoCapture &cap);

    public:
        IRController();
        ~IRController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
};

#endif