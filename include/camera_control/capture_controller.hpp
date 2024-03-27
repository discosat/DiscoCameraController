#ifndef CAPTURE_CONTROLLER_H
#define CAPTURE_CONTROLLER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "camera_controller.hpp"
#include "vimba_controller.hpp"
#include "common.hpp"
#include "message_queue.hpp"

// capture parameters defaults
#define EXPOSURE_DEFAULT 0
#define ISO_DEFAULT 0
#define NUM_IMAGES_DEFAULT 1
#define INTERVAL_DEFAULT 0
#define CAMERA_TYPE_DEFAULT CameraType::VMB

class CaptureController{
    public:
        CaptureController();
        ~CaptureController();

        void Capture(CaptureMessage capture_instructions, u_int16_t* error);

        static void CaptureCallback(char* capture_instructions, void* obj, u_int16_t* error) {
            if (obj){
                std::string input(capture_instructions);
                CaptureMessage msg = ParseMessage(input);
                static_cast<CaptureController*>(obj)->Capture(msg, error);
            }
        }

        static std::unique_ptr<CameraController> CreateControllerInstance(CameraType type){
            switch (type)
            {
            case CameraType::VMB:
                return std::make_unique<VimbaController>();
                break;
            
            case CameraType::IR:
                // TODO!: add ir camera class implementation
                return nullptr;
                break;
            
            case CameraType::Unkown:
                return nullptr;
                break;
            
            default:
                return std::make_unique<VimbaController>();
                break;
            }
        }

        static CaptureMessage ParseMessage(const std::string& input) {
            CaptureMessage message;
            message.Exposure = EXPOSURE_DEFAULT;
            message.ISO = ISO_DEFAULT;
            message.Interval = INTERVAL_DEFAULT;
            message.NumberOfImages = NUM_IMAGES_DEFAULT;
            message.CameraId = "";
            message.Type = CAMERA_TYPE_DEFAULT;

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
                } else if (variable == "INTERVAL") {
                    message.Interval = std::stoi(value);
                } else if (variable == "EXPOSURE") {
                    message.Exposure = std::stoi(value);
                } else if (variable == "CAMERA_ID") {
                    message.CameraId = std::string(value);
                } else if (variable == "CAMERA_TYPE") {
                    message.Type = StringToCameraType(value);
                } else {
                    continue;
                }
            }
            return message;
        }
    
    private:
        MessageQueue* mq;
};

#endif