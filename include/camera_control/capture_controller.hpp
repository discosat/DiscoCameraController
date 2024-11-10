#ifndef CAPTURE_CONTROLLER_H
#define CAPTURE_CONTROLLER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

#include "camera_controller.hpp"
#include "vimba_controller.hpp"
#include "test_controller.hpp"
#include "ir_controller.hpp"
#include "common.hpp"
#include "errors.hpp"
#include "message_queue.hpp"

// capture parameters defaults
#define EXPOSURE_DEFAULT 0
#define ISO_DEFAULT 0
#define NUM_IMAGES_DEFAULT 1
#define INTERVAL_DEFAULT 0
#define CAMERA_TYPE_DEFAULT CameraType::VMB
#define PIPELINE_ID_DEFAULT 1

// auto exposure paramteres
const float MAX_EXPOSURE = 150000; // maximum allowed exposure
const float MAX_ENTROPY = 8; // maximum entropy achieved by 8 bits
const size_t STEPS = 8;
const float EXPOSURE_START = 5000;
const float LEARNING_RATE = 500;

class CaptureController{
    public:
        CaptureController();
        ~CaptureController();

        void Capture(CaptureMessage capture_instructions, u_int16_t* error);

        static void CaptureCallback(char* capture_instructions, void* obj, u_int16_t* error) {
            if (obj){
                std::string input(capture_instructions);
                if(input.size() == 0){
                    *error = ERROR_CODE::PARSING_ERROR_MESSAGE_EMPTY;
                    return;
                }

                CaptureMessage msg = ParseMessage(input);

                try{
                    static_cast<CaptureController*>(obj)->Capture(msg, error);
                } catch(std::exception const&){
                    *error = ERROR_CODE::CAPTURE_ERROR;
                }
            }
        }

        static std::unique_ptr<CameraController> CreateControllerInstance(CameraType type){
            switch (type)
            {
            case CameraType::VMB:
                return std::make_unique<VimbaController>();
                break;
            
            case CameraType::IR:
                return std::make_unique<IRController>();
                break;

            case CameraType::TEST:
                return std::make_unique<TestController>();
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
            message.PipelineId = PIPELINE_ID_DEFAULT;
            message.OBID = 0;

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
                } else if (variable == "PIPELINE_ID") {
                    message.PipelineId = std::stoi(value);
                } else if (variable == "OBID") {
                    message.OBID == std::stoi(value);
                } else {
                    continue;
                }
            }
            return message;
        }
    
    private:
        MessageQueue* mq;

        uchar* createImageMessageData(std::vector<Image> &images, CaptureMessage capture_instructions, size_t &size);
        double calculateEntropy(Image image);
        size_t setExposure(CameraController *controller, CaptureMessage cap_msg);
};

#endif