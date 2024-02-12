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
        VmbCPP::FramePtr frame = p->AqcuireFrame(cam, currentExposure, 0);            
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

Optional Arguments:
  -s             Save the processed images.
  -o PATH        Output path to save the images.
  -n NUM_IMAGES  Specify the number of images to save (only valid with -s).
  -f FEATURES    Output a comma-separated list of features.
    )""";

    std::cout << help << std::endl;
}

int main(int argc, char *argv[], char *envp[]){
    // parse arguments
    const std::vector<std::string_view> args(argv, argv + argc);
    const std::string_view exposure_arg = get_option(args, "-e");
    const bool save_images = has_option(args, "-s");
    const std::string_view image_out = get_option(args, "-o");
    const std::string_view num_images_arg = get_option(args, "-n");
    const std::string_view feature_outputs_arg = get_option(args, "-f");

    // parsed arguments
    float exposure = 0;
    int num_images = 1;
    std::vector<std::string> feature_outputs = split_string(std::string(feature_outputs_arg));

    if(has_option(args, "-h")){
        print_usage();
        return 0;
    }

    if(save_images && image_out.size() == 0){
        std::cerr << "Define the output path to save the images to" << std::endl;
        print_usage();
        return 1;
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

    VimbaProvider* vmbProvider = new VimbaProvider();
    MessageQueue* mq = new MessageQueue();
    std::vector<VmbCPP::CameraPtr> cameras = vmbProvider->GetCameras();

    if(cameras.size() > 0){
        cameras.at(0)->Open(VmbAccessModeExclusive);
        exposure = (exposure == 0)?set_exposure(cameras.at(0), vmbProvider):exposure;

        for(int i = 0; i < num_images; i++){
            VmbCPP::FramePtr frame = vmbProvider->AqcuireFrame(cameras.at(0), exposure, 0);

            if(save_images){
                // get image data
                unsigned char* buffer;
                frame->GetImage(buffer);

                // get image dimensions
                unsigned int width, height;
                frame->GetHeight(height);
                frame->GetWidth(width);

                // convert to open CV matrix and color correct
                cv::Mat img(height, width, CV_8UC3, buffer);
                cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

                ImageBatch batch;
                batch.mtype = 0;
                batch.height = height;
                batch.width = width;
                batch.channels = 3;
                batch.num_images = 1;
                batch.data_size = 3*width*height;
                batch.data = buffer;

                if(mq->SendImage(batch)){
                    std::cout << "Sending image was successful" << std::endl;
                } else {
                    std::cout << "Sending image was unsuccessful" << std::endl;
                }

                // // save to path
                // fs::path dir (image_out);
                // fs::path file ("image_" + std::to_string(std::time(0)) + "_" + std::to_string(exposure) + ".png");
                // std::string full_path = (dir / file).string();
                // imwrite(full_path, img);
            }
        }

        cameras.at(0)->Close();
    }

    delete vmbProvider; 
    delete mq;
    return 0;
}