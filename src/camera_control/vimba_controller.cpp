#include "vimba_controller.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
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

FramePtr VimbaController::aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain){
    if(!turnOnCamera(cam)){
        throw std::runtime_error("Could not turn on camera");
    }

    FeaturePtr pFormatFeature;
    VmbErrorType err = cam->GetFeatureByName( "PixelFormat", pFormatFeature );
    
    if ( VmbErrorSuccess == err )
    {
        err = pFormatFeature->SetValue( VmbPixelFormatBayerRG12 );
        if ( VmbErrorSuccess != err ) {
            pFormatFeature->SetValue( VmbPixelFormatBayerGR12 );
        }
        
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

    FramePtr frame;
    err = cam->AcquireSingleImage(frame, 5000);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    if(!turnOffCamera(cam)){
        std::cerr << "Could not turn off camera" << std::endl;
    }

    return frame;
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
        
        for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
            VmbCPP::FramePtr frame;
            size_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            try{
                frame = this->aqcuireFrame(cam, capture_instructions.Exposure, capture_instructions.ISO);
            } catch(const std::exception& e){
                std::cout << "Error!" << std::endl;
                *error = ERROR_CODE::CAPTURE_ERROR;
                return images;
            }

            u_int width, height, bufferSize;
            frame->GetBufferSize(bufferSize);
            frame->GetWidth(width);
            frame->GetHeight(height);

            u_char* buffer;
            frame->GetImage(buffer);

            Image img;
            img.size = bufferSize;
            img.width = width;
            img.height = height;
            img.data = new u_char[bufferSize];
            img.bpp = VMB_BPP;
            img.channels = VMB_CHANNELS;
            img.timestamp = timestamp;

            std::memcpy(img.data, buffer, bufferSize * sizeof(u_char));

            images.push_back(img);

            // delay if needed
            if(i < capture_instructions.NumberOfImages - 1 && capture_instructions.Interval > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(capture_instructions.Interval));
            }
        }

        cam->Close();
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_CAMERA_NOT_FOUND;
    }

    return images;
}