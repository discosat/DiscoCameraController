#include "test_controller.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include "common.hpp"

TestController::TestController(){}

TestController::~TestController(){};

std::vector<Image> TestController::Capture(CaptureMessage& capture_instructions, u_int16_t* error){
    std::vector<Image> images;

    for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
        std::ifstream file(TEST_FILE_LOCATION, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;
            return images;
        }
        
        file.seekg(0, std::ios::end);
        size_t fileSize = (size_t)file.tellg();
        file.seekg(0, std::ios::beg);

        char* buffer = new char[fileSize];
        file.read(buffer, fileSize);

        if (!file) {
            std::cerr << "Failed to read the file." << std::endl;
            delete[] buffer;
            return images;
        }

        file.close();

        Image img;
        img.bpp = TEST_BPP;
        img.channels = TEST_TEST_CHANNELS;
        img.width = TEST_WIDTH;
        img.height = TEST_HEIGHT;
        img.timestamp = 15;
        img.size = fileSize;
        img.data = (uchar*)buffer;
        images.push_back(img);
    }

    return images;
}