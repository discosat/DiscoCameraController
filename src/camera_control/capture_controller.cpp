#include "capture_controller.hpp"
#include <iostream>
#include <memory>
#include "errors.hpp"

CaptureController::CaptureController(){
    this->mq = new MessageQueue();
}

CaptureController::~CaptureController(){
    delete this->mq;
}

void CaptureController::Capture(CaptureMessage capture_instructions, u_int16_t* error){
    if(capture_instructions.NumberOfImages == 0){
        *error = ERROR_CODE::PARSING_ERROR_NUM_IMAGES_INVALID;
        return;
    }
    
    std::cout << "New capture instructions:" << std::endl;
    std::cout << "\tCamera: \"" << capture_instructions.CameraId << "\"" << std::endl;
    std::cout << "\tExposure: " << capture_instructions.Exposure << std::endl;
    std::cout << "\tISO: " << capture_instructions.ISO << std::endl;
    std::cout  << "\tNumber of images: " << capture_instructions.NumberOfImages << std::endl;
    
    std::unique_ptr<CameraController> controller = CaptureController::CreateControllerInstance(capture_instructions.Type);

    if(controller == nullptr){
        *error = ERROR_CODE::PARSING_ERROR_CAMERA_TYPE_INVALID;
        return;
    }

    auto images = controller->Capture(capture_instructions, error);

    if(*error != ERROR_CODE::SUCCESS){
        return;
    }

    u_int image_size = images.at(0).size, width = images.at(0).width, height = images.at(0).height;
    u_int bufferSize = image_size + IMAGE_METADATA_SIZE;
    unsigned char* total_buffer = new unsigned char[bufferSize*capture_instructions.NumberOfImages];

    for(size_t i = 0; i < images.size(); i++){
        unsigned int offset = i*bufferSize;

        std::memcpy((void*)(&total_buffer[offset]), &image_size, sizeof(unsigned int));
        std::memcpy((void*)(&total_buffer[offset+IMAGE_METADATA_SIZE]), images.at(i).data, image_size * sizeof(unsigned char));
    }

    ImageBatch batch;
    batch.height = height;
    batch.width = width;
    batch.channels = 1;
    batch.num_images = capture_instructions.NumberOfImages;
    batch.batch_size = bufferSize*capture_instructions.NumberOfImages;
    batch.data = total_buffer;

    if(mq->SendImage(batch, error)){
        std::cout << "Sending image was successful" << std::endl;
    } else {
        std::cout << "Sending image was unsuccessful" << std::endl;
    }
}