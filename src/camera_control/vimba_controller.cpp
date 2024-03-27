#include <iostream>
#include <stdexcept>
#include <VmbCPP/VmbCPP.h>
#include "vimba_controller.hpp"
#include "errors.hpp"
#include <memory>

using namespace VmbCPP;

VimbaController::VimbaController() : sys(VmbSystem::GetInstance()){
    VmbErrorType err = this->sys.Startup();

    if (VmbErrorSuccess != err)
    {
        throw std::runtime_error("Could not start system. Error code: " + std::to_string(err) );
    }
}

VimbaController::~VimbaController(){
    sys.Shutdown();
}

std::vector<CameraPtr> VimbaController::GetCameras(){
    CameraPtrVector cams;
    VmbErrorType err = sys.GetCameras(cams);

    if (VmbErrorSuccess == err)
    {
        return cams;
    }
    else
    {
        sys.Shutdown();
        throw std::runtime_error("Could not list cameras. Error code: " + std::to_string(err) );
    }
}

FramePtrVector VimbaController::AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain, int numFrames){
    FeaturePtr pFormatFeature;

    VmbErrorType err = cam->GetFeatureByName( "PixelFormat", pFormatFeature );
    
    if ( VmbErrorSuccess == err )
    {
        err = pFormatFeature->SetValue( VmbPixelFormatBayerGR12 );
        FeaturePtr pExposureFeature, pGainFeature;
        
        err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
        if ( VmbErrorSuccess != err ) {
            err = cam->GetFeatureByName("ExposureTime", pExposureFeature);
        }

        if (VmbErrorSuccess == err) {
            err = pExposureFeature->SetValue(exposure);
        }

        err = cam->GetFeatureByName("Gain", pGainFeature);

        if (VmbErrorSuccess == err) {
            err = pGainFeature->SetValue(gain);
        }
    }

    FramePtrVector frames;

    for(int i = 0; i < numFrames; i++){
        FramePtr frame;
        frames.push_back(frame);
    }

    err = cam->AcquireMultipleImages(frames, 5000);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    return frames;
}

std::vector<Image> VimbaController::Capture(CaptureMessage& capture_instructions, u_int16_t* error) {
    std::vector<VmbCPP::CameraPtr> cameras = this->GetCameras();
    VmbCPP::CameraPtr cam;
    std::vector<Image> images;
    
    if(cameras.size() > 0){
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);

            if(camName == capture_instructions.CameraId){
                cam = cameras.at(i);
            }
        }
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_NO_CAMERAS;
        return images;
    }

    if(cam != NULL){
        cam->Open(VmbAccessModeExclusive);
        float exposure = capture_instructions.Exposure;
        VmbCPP::FramePtrVector frames;

        try{
            frames = this->AqcuireFrame(cam, exposure, capture_instructions.ISO, capture_instructions.NumberOfImages);
        } catch(const std::exception& e){
            *error = ERROR_CODE::CAPTURE_ERROR;
            return images;
        }
        
        u_int width, height, bufferSize;
        frames.at(0)->GetBufferSize(bufferSize);

        bufferSize+=IMAGE_METADATA_SIZE;

        frames.at(0)->GetWidth(width);
        frames.at(0)->GetHeight(height);

        for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
            u_char* buffer;
            frames.at(i)->GetImage(buffer);
            Image img;
            img.size = bufferSize;
            img.width = width;
            img.height = height;
            img.data = buffer;

            images.push_back(img);
        }

        cam->Close();
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_CAMERA_NOT_FOUND;
    }

    return images;
}