#include "ir_controller.hpp"
#include <opencv2/opencv.hpp>

IRController::IRController(){}

IRController::~IRController(){};

bool IRController::openCamera(cv::VideoCapture &cap,std::string port_name){
    cap = cv::VideoCapture(port_name, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter().fourcc('Y','1','6',' '));
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);

    if (!cap.isOpened()) {
        return false;
    }
    return true;
}

void IRController::closeCamera(cv::VideoCapture &cap){
    cv::destroyAllWindows();
    cap.release();
}

u_char* IRController::captureFrame(cv::VideoCapture &cap){
    cv::Mat frame;
    if (cap.read(frame)) {
        u_char* out_array = new u_char[BUFFERSIZE];
        memcpy(out_array, (u_char*)frame.data, BUFFERSIZE);
        return out_array;
    } else {
        return nullptr;
    }
}

std::vector<Image> IRController::Capture(CaptureMessage& capture_instructions, u_int16_t* error){
    std::vector<Image> images;
    std::string camera_name = "/dev/v4l/by-id/usb-FLIR_Boson_267763-video-index0";

    cv::VideoCapture cap;
    if (!openCamera(cap, camera_name)){

    }

    for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
        Image img;
        img.size = BUFFERSIZE;
        img.width = WIDTH;
        img.height = HEIGHT;
        img.data = captureFrame(cap);
        img.bpp = IR_BPP;

        images.push_back(img);
    }

    closeCamera(cap);
    return images;
}