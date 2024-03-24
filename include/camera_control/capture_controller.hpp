#ifndef CAPTURE_CONTROLLER_H
#define CAPTURE_CONTROLLER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "vimba_controller.hpp"
#include "common.hpp"
#include "message_queue.hpp"

class CaptureController{
    public:
        CaptureController();
        ~CaptureController();

        void Capture(CaptureMessage capture_instructions);

        static void CaptureCallback(char* capture_instructions, void* obj) {
            if (obj){
                CaptureMessage msg;
                std::string input(capture_instructions);
                ParseMessage(input, msg);
                static_cast<CaptureController*>(obj)->Capture(msg);
            }
        }

        static void ParseMessage(const std::string& input, CaptureMessage& message) {
            std::vector<std::string> pairs;
            std::stringstream ss(input);
            std::string pair;
            while (std::getline(ss, pair, ';')) {
                pairs.push_back(pair);
            }

            for (const auto& p : pairs) {
                std::istringstream iss(p);
                std::string variable, value;
                std::getline(iss, variable, '=');
                std::getline(iss, value);
                
                variable.erase(0, variable.find_first_not_of(" \t\n\r\f\v"));
                variable.erase(variable.find_last_not_of(" \t\n\r\f\v") + 1);
                value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
                value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
                
                // Check variable name and assign value -> kinda dirty...
                if (variable == "NUM_IMAGES") {
                    message.NumberOfImages = std::stoi(value);
                } else if (variable == "ISO") {
                    message.ISO = std::stof(value);
                } else if (variable == "EXPOSURE") {
                    message.Exposure = std::stoi(value);
                } else if (variable == "CAMERA") {
                    message.Camera = value.c_str();
                } else {
                    continue;
                }
            }
        }
    
    private:
        VimbaController* vmbProvider;
        MessageQueue* mq;

        ImageBatch CaptureVimba(CaptureMessage params, VmbCPP::CameraPtr cam);
};

#endif