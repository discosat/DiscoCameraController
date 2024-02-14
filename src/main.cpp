#include <iostream>
#include <vector>
#include <VmbCPP/VmbCPP.h>
#include <opencv2/opencv.hpp>
#include "vimba_provider.hpp"
#include "exposure_helper.hpp"
#include "types.hpp"
#include <filesystem>
#include <ctime>
#include <bits/stdc++.h>
#include <chrono>
#include "message_queue.hpp"

namespace fs = std::filesystem;

using namespace Disco2Camera;
using namespace std::chrono;

// auto exposure paramteres
const float MAX_EXPOSURE = 1000000.0; // maximum allowed exposure
const float MAX_ENTROPY = 400.923; // maximum entropy achieved by a image with 3 channels
const size_t STEPS = 20;
const float MIN_EXPOSURE = 10000.0;
const float LEARNING_RATE = 20000;

float set_exposure(VmbCPP::CameraPtr cam, VimbaProvider* p){
    float currentExposure = MIN_EXPOSURE, lastEntropy = -1, lastExposure = -1, slope = 1;
    size_t steps = 0;

    while(currentExposure < MAX_EXPOSURE && steps < STEPS){
        VmbCPP::FramePtrVector frames = p->AqcuireFrame(cam, currentExposure, 0, 1);            
        VmbCPP::FramePtr frame = frames.at(0);
        unsigned int size, width, height;
        frame->GetBufferSize(size);
        frame->GetHeight(height);
        frame->GetWidth(width);
        
        unsigned char* buffer;
        frame->GetImage(buffer);

        cv::Mat img(height, width, CV_8UC3, buffer);
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        float currentEntropy = ExposureHelper::calculateEntropy(img);

        if(lastEntropy == -1){
            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE;
        } else {
            float exposure_delta = currentExposure - lastExposure;
            float entropy_delta = currentEntropy - lastEntropy;
            slope = (entropy_delta/MAX_ENTROPY)/(exposure_delta/MAX_EXPOSURE); // normalized slope

            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE*slope;
            std::cout << slope << " " << currentExposure << std::endl;
        }

        steps++;
    }

    return currentExposure;
}

std::string_view get_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }
    
    return "";
}

bool has_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            return true;
    }
    
    return false;
}

std::vector<std::string> split_string(std::string input){
    std::istringstream iss(input);
    std::string word;
    std::vector<std::string> words;

    while (std::getline(iss, word, ',')) {
        words.push_back(word);
    }

    return words;
}

void print_usage(){
    std::string help = R"""(Usage: Disco2CameraControl -e EXPOSURE [-s] [-n NUM_IMAGES] [-f FEATURES]

Description:
  Perform image processing with the following options:

Required Argument:
  -e EXPOSURE   Set the exposure level (a positive number) or auto.
  -c CAMERA      Specify the camera to be used in the current burst.

Optional Arguments:
  -n NUM_IMAGES  Specify the number of images to save (only valid with -s), default is 1.
    )""";

    std::cout << help << std::endl;
}

int main(int argc, char *argv[], char *envp[]){
    // parse arguments
    const std::vector<std::string_view> args(argv, argv + argc);
    const std::string_view exposure_arg = get_option(args, "-e");
    const std::string_view camera = get_option(args, "-c");
    const std::string_view num_images_arg = get_option(args, "-n");

    VimbaProvider* vmbProvider = new VimbaProvider();
    MessageQueue* mq = new MessageQueue();
    std::vector<VmbCPP::CameraPtr> cameras = vmbProvider->GetCameras();
    VmbCPP::CameraPtr cam;

    // parsed arguments
    float exposure = 0;
    int num_images = 1;

    if(has_option(args, "-h")){
        print_usage();
        return 0;
    }

    if(exposure_arg.size() > 0){
        exposure = (exposure_arg == "auto")? 0 : std::atof(std::string(exposure_arg).c_str());
    } else {
        print_usage();
        return 1;
    }

    if(num_images_arg.size() > 0){
        num_images = std::atoi(std::string(num_images_arg).c_str());
    }

    if(cameras.size() > 0){
        for(int i = 0; i < cameras.size(); i++){
            std::string camName = "";
            cameras.at(i)->GetModel(camName);

            if(camName == camera){
                cam = cameras.at(i);
            }
        }
    } else {
        std::cerr << "No cameras were detected" << std::endl;
        return 1;
    }

    std::cout << "Size of pointer: " << sizeof(int*) << " bytes" << std::endl;

    if(cam != NULL && num_images > 0){
        cam->Open(VmbAccessModeExclusive);
        exposure = (exposure == 0)?set_exposure(cam, vmbProvider):exposure;
        VmbCPP::FramePtrVector frames = vmbProvider->AqcuireFrame(cameras.at(0), exposure, 0, num_images);
        
        unsigned int width, height, bufferSize;
        frames.at(0)->GetBufferSize(bufferSize);
        frames.at(0)->GetWidth(width);
        frames.at(0)->GetHeight(height);

        unsigned char* total_buffer = new unsigned char[bufferSize*num_images];

        for(int i = 0; i < num_images; i++){
            unsigned char* buffer;
            frames.at(i)->GetImage(buffer);
            std::memcpy((void*)(&total_buffer[i * bufferSize]), buffer, bufferSize * sizeof(unsigned char));
        }

        ImageBatch batch;
        batch.height = height;
        batch.width = width;
        batch.channels = 3;
        batch.num_images = num_images;
        batch.data_size = bufferSize*num_images;
        batch.data = total_buffer;

        if(mq->SendImage(batch)){
            std::cout << "Sending image was successful" << std::endl;
        } else {
            std::cout << "Sending image was unsuccessful" << std::endl;
        }
        
        delete[] total_buffer;
        cam->Close();
    } else {
        if(num_images <= 0){
            std::cerr << "Number of images must be greater than zero" << std::endl;
        }

        if(cam == NULL){
            std::cerr << "Camera must be one of: ";

            for(int i = 0; i < cameras.size(); i++){
                std::string camName;
                cameras.at(i)->GetModel(camName);
                std::cerr << camName << " ";
            }

            std::cerr << std::endl;
        }
    }

    delete vmbProvider; 
    delete mq;
    return 0;
}