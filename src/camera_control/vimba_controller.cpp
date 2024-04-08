#include "vimba_controller.hpp"
#include <iostream>
#include <stdexcept>
#include <VmbCPP/VmbCPP.h>
#include "errors.hpp"
#include "common.hpp"
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

std::vector<CameraPtr> VimbaController::getCameras(){
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

bool VimbaController::turnOnCamera(VmbCPP::CameraPtr cam){
    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName( "DevicePowerSavingMode", powerSavingFeature );

    if ( VmbErrorSuccess == err ){
        err = powerSavingFeature->SetValue("Disabled");
        std::cout << err << std::endl;
        return VmbErrorSuccess == err;
    } else {
        std::cout << err << std::endl;
        return false;
    }
}

bool VimbaController::turnOffCamera(VmbCPP::CameraPtr cam){
    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName( "DevicePowerSavingMode", powerSavingFeature );

    if ( VmbErrorSuccess == err ){
        err = powerSavingFeature->SetValue("SuspendMode");
        return VmbErrorSuccess == err;
    } else {
        return false;
    }
}

bool VimbaController::setDelay(VmbCPP::CameraPtr cam, uint delay){
    double hertz = ((double)1.0)/(((double)delay)/1000000);
    
    FeaturePtr acquisitionFrameRateEnable, acquisitionFrameRate;
    VmbErrorType err = VmbErrorSuccess;

    if ((err = cam->GetFeatureByName( "AcquisitionFrameRateEnable", acquisitionFrameRateEnable )) == VmbErrorSuccess ){
        err = acquisitionFrameRateEnable->SetValue(true);

        if(err == VmbErrorSuccess && 
            (err = cam->GetFeatureByName( "AcquisitionFrameRate", acquisitionFrameRate )) == VmbErrorSuccess ){
                err = acquisitionFrameRate->SetValue((float)hertz);
                return VmbErrorSuccess == err;
            }
    }
    return true;
}

FramePtrVector VimbaController::aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain, uint numFrames, uint delay){
    if(!turnOnCamera(cam)){
        throw std::runtime_error("Could not turn on camera");
    }

    if(delay > 0){
        setDelay(cam, delay);
    }

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

    for(uint i = 0; i < numFrames; i++){
        FramePtr frame;
        frames.push_back(frame);
    }

    err = cam->AcquireMultipleImages(frames, 5000);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    if(!turnOffCamera(cam)){
        std::cerr << "Could not turn off camera" << std::endl;
    }

    return frames;
}

std::vector<Image> VimbaController::Capture(CaptureMessage& capture_instructions, u_int16_t* error) {
    std::vector<VmbCPP::CameraPtr> cameras = this->getCameras();
    VmbCPP::CameraPtr cam;
    std::vector<Image> images;
    
    if(cameras.size() > 0){
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);
            std::cout << "Camera found: " << camName << std::endl;

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
            std::cout << "endl" << std::endl;
            frames = this->aqcuireFrame(cam, exposure, capture_instructions.ISO, capture_instructions.NumberOfImages, capture_instructions.Interval);
        } catch(const std::exception& e){
            std::cout << "Error!" << std::endl;
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
            img.data = new u_char[bufferSize];
            img.bpp = VMB_BPP;

            std::memcpy(img.data, buffer, bufferSize * sizeof(u_char));

            images.push_back(img);
        }

        cam->Close();
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_CAMERA_NOT_FOUND;
    }

    return images;
}