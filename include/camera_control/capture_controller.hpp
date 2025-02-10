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

        static void CaptureCallback(char *camera_id, uint8_t camera_type, uint32_t exposure, double iso, uint32_t num_images, uint32_t interval, uint32_t obid, uint32_t pipeline_id, void* obj, u_int16_t* error) {
            if (obj){

                CaptureMessage msg = CreateCaptureMessage(camera_id, camera_type, exposure, iso, num_images, interval, obid, pipeline_id);

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
            
            case CameraType::Unknown:
                return nullptr;
                break;
            
            default:
                return std::make_unique<VimbaController>();
                break;
            }
        }

        static CaptureMessage CreateCaptureMessage(char *camera_id, uint8_t camera_type, uint32_t exposure, double iso, uint32_t num_images, uint32_t interval, uint32_t obid, uint32_t pipeline_id) {
            CaptureMessage message;
            message.Exposure = exposure;
            message.ISO = iso;
            message.Interval = interval;
            message.NumberOfImages = num_images;
            message.CameraId = std::string(camera_id);
            message.Type = IntToCameraType(camera_type);
            message.PipelineId = pipeline_id;
            message.OBID = obid;

            return message;
        }
    
    private:
        MessageQueue* mq;

        uchar* createImageMessageData(std::vector<Image> &images, CaptureMessage capture_instructions, size_t &size);
        double calculateEntropy(Image image);
        size_t setExposure(CameraController *controller, CaptureMessage cap_msg);
};

#endif