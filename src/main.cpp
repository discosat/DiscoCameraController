#include <iostream>
#include <vector>
#include <VmbCPP/VmbCPP.h>
#include <opencv2/opencv.hpp>
#include "camera_control/camera_controller.hpp"
#include "utils/exposure_helper.hpp"
#include "utils/common.hpp"
#include <filesystem>
#include <ctime>
#include <bits/stdc++.h>
#include <chrono>
#include "communication/message_queue.hpp"
#include "communication/csp_server.hpp"

namespace fs = std::filesystem;

using namespace std::chrono;

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
    std::string help = R"""(Usage: Disco2CameraControl -i INTERFACE -p DEVICE -a NODE

Description:
  Listen to a capture message with the following options:

Arguments:
  -i INTERFACE   The connection interface to utilize, can be one of: zmq, can, kiss.
                 Default: zmq
  -d DEVICE      Device connection interface.
                 Default: localhost
  -p PORT        Port that the server listens to for command packets.
                 Default: 10
  -n NODE        CSP node address.
                 Default: 2
)""";

    std::cout << help << std::endl;
}

void capture(CaptureMessage params, CameraController* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras){
    VmbCPP::CameraPtr cam;
    
    if(cameras.size() > 0){
        for(int i = 0; i < cameras.size(); i++){
            std::string camName = "";
            cameras.at(i)->GetModel(camName);

            if(camName == params.Camera){
                cam = cameras.at(i);
            }
        }
    } else {
        std::cerr << "No cameras were detected" << std::endl;
        return;
    }

    std::cout << "Size of pointer: " << sizeof(int*) << " bytes" << std::endl;

    if(cam != NULL && params.NumberOfImages > 0){
        cam->Open(VmbAccessModeExclusive);
        float exposure = (params.Exposure == 0)?set_exposure(cam, vmbProvider):params.Exposure;
        VmbCPP::FramePtrVector frames = vmbProvider->AqcuireFrame(cameras.at(0), exposure, 0, params.NumberOfImages);
        
        unsigned int width, height, bufferSize, imageSize;
        frames.at(0)->GetBufferSize(bufferSize);

        imageSize = bufferSize;
        bufferSize+=IMAGE_METADATA_SIZE;

        frames.at(0)->GetWidth(width);
        frames.at(0)->GetHeight(height);

        unsigned char* total_buffer = new unsigned char[bufferSize*params.NumberOfImages];

        for(int i = 0; i < params.NumberOfImages; i++){
            unsigned char* buffer;
            unsigned int offset = i*bufferSize;
            frames.at(i)->GetImage(buffer);

            // copy the image 
            std::memcpy((void*)(&total_buffer[offset]), &imageSize, sizeof(unsigned int));
            std::memcpy((void*)(&total_buffer[offset+IMAGE_METADATA_SIZE]), buffer, imageSize * sizeof(unsigned char));
        }

        ImageBatch batch;
        batch.height = height;
        batch.width = width;
        batch.channels = 1;
        batch.num_images = params.NumberOfImages;
        batch.batch_size = bufferSize*params.NumberOfImages;
        batch.data = total_buffer;

        if(mq->SendImage(batch)){
            std::cout << "Sending image was successful" << std::endl;
        } else {
            std::cout << "Sending image was unsuccessful" << std::endl;
        }
        
        delete[] total_buffer;
        cam->Close();
    } else {
        if(params.NumberOfImages <= 0){
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
}

int main(int argc, char *argv[], char *envp[]){
    // parse arguments
    const std::vector<std::string_view> args(argv, argv + argc);
    
    if(has_option(args, "-h")){
        print_usage();
        return 0;
    }
    
    const std::string_view interface_arg = get_option(args, "-i");
    const std::string_view device_arg = get_option(args, "-d");
    const std::string_view node_arg = get_option(args, "-n");
    const std::string_view port_arg = get_option(args, "-p");
    int node = 2, port = 10;
    std::string device = "localhost", interface = "zmq";

    if(node_arg.size() > 0){
        node = std::atoi(std::string(node_arg).c_str());
    }

    if(port_arg.size() > 0){
        port = std::atoi(std::string(port_arg).c_str());
    }

    if(device_arg.size() > 0){
        device = std::string(device_arg);
    }

    if(interface_arg.size() > 0){
        interface = std::string(interface_arg);
    }

    CSPInterface interfaceConfig;
    interfaceConfig.Interface = StringToCSPInterface(interface);
    interfaceConfig.Device = std::string(device_arg);
    interfaceConfig.Node = node;
    interfaceConfig.Port = port;

    CameraController* vmbProvider = new CameraController();
    MessageQueue* mq = new MessageQueue();
    std::vector<VmbCPP::CameraPtr> cameras = vmbProvider->GetCameras();

    server_init(capture, vmbProvider, mq, cameras, interfaceConfig);

    delete vmbProvider; 
    delete mq;
    return 0;
}