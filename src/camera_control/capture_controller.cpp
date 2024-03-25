#include "capture_controller.hpp"
#include <iostream>

CaptureController::CaptureController(){
    this->mq = new MessageQueue();
    this->vmbProvider = new VimbaController();
}

CaptureController::~CaptureController(){
    delete this->mq;
    delete this->vmbProvider;
}

ImageBatch CaptureController::CaptureVimba(CaptureMessage params, VmbCPP::CameraPtr cam){
    cam->Open(VmbAccessModeExclusive);
    float exposure = (params.Exposure == 0)?55000:params.Exposure;//set_exposure(cam, vmbProvider):params.Exposure;
    VmbCPP::FramePtrVector frames = this->vmbProvider->AqcuireFrame(cam, exposure, params.ISO, params.NumberOfImages);
    
    unsigned int width, height, bufferSize, imageSize;
    frames.at(0)->GetBufferSize(bufferSize);

    imageSize = bufferSize;
    bufferSize+=IMAGE_METADATA_SIZE;

    frames.at(0)->GetWidth(width);
    frames.at(0)->GetHeight(height);

    unsigned char* total_buffer = new unsigned char[bufferSize*params.NumberOfImages];

    for(size_t i = 0; i < params.NumberOfImages; i++){
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

    cam->Close();
    return batch;
}

void CaptureController::Capture(CaptureMessage capture_instructions){
    std::cout << "New capture instructions:" << std::endl;
    std::cout << "\tCamera: \"" << capture_instructions.Camera << "\"" << std::endl;
    std::cout << "\tExposure: " << capture_instructions.Exposure << std::endl;
    std::cout << "\tISO: " << capture_instructions.ISO << std::endl;
    std::cout  << "\tNumber of images: " << capture_instructions.NumberOfImages << std::endl;

    std::vector<VmbCPP::CameraPtr> cameras = vmbProvider->GetCameras();
    VmbCPP::CameraPtr cam;
    
    if(cameras.size() > 0){
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);

            if(camName == capture_instructions.Camera){
                cam = cameras.at(i);
            }
        }
    } else {
        std::cerr << "No cameras were detected" << std::endl;
        return;
    }

    if(cam != NULL && capture_instructions.NumberOfImages > 0){
        ImageBatch batch = this->CaptureVimba(capture_instructions, cam);
        
        if(mq->SendImage(batch)){
            std::cout << "Sending image was successful" << std::endl;
        } else {
            std::cout << "Sending image was unsuccessful" << std::endl;
        }
        
        delete[] batch.data;
    } else {
        if(capture_instructions.NumberOfImages <= 0){
            std::cerr << "Number of images must be greater than zero" << std::endl;
        }

        if(cam == NULL){
            std::cerr << "Camera must be one of: ";

            for(size_t i = 0; i < cameras.size(); i++){
                std::string camName;
                cameras.at(i)->GetModel(camName);
                std::cerr << camName << " ";
            }

            std::cerr << std::endl;
        }
    }
}