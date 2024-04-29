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
        std::cout << BUFFERSIZE << " " << frame.total() * frame.elemSize() << std::endl;
        size_t bufferSize = frame.total() * frame.elemSize();
        uchar* out_array = new uchar[bufferSize];
        std::memcpy(out_array, frame.data, bufferSize);
        return out_array;
    } else {
        return nullptr;
    }
}

std::vector<Image> IRController::Capture(CaptureMessage& capture_instructions, u_int16_t* error){
    std::vector<Image> images;
    std::string camera_name = capture_instructions.CameraId.size() == 0?
        IR_CAMERA_DEFAULT_ID:capture_instructions.CameraId;

    cv::VideoCapture cap;
    if (!openCamera(cap, capture_instructions.CameraId)){
        std::cerr << "Unable to open IR camera" << std::endl;
        return images;
    }

    std::cerr << "Opened IR camera" << std::endl;

    for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
        std::cerr << "Capturing IR image" << std::endl;
        Image img;
        img.size = BUFFERSIZE;
        img.width = WIDTH;
        img.height = HEIGHT;
        img.data = captureFrame(cap);
        img.bpp = IR_BPP;

        images.push_back(img);
    }


    closeCamera(cap);

    std::cout << "Captured IR frames" << std::endl; 
    return images;
}