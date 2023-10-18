#include <iostream>
#include <stdexcept>
#include <VmbCPP/VmbCPP.h>
#include "VimbaProvider.hpp"

using namespace VmbCPP;

Disco2Camera::VimbaProvider::VimbaProvider() : sys(VmbSystem::GetInstance()){
    VmbErrorType err = sys.Startup();

    if (err != VmbErrorSuccess)
    {
        /* code */
    }
    
    // this->cameras = GetCameras();

    // for(CameraPtr cam : cameras){
    //     err = cam->Open(VmbAccessModeExclusive);
    // }

    if (VmbErrorSuccess != err)
    {
        throw std::runtime_error("Could not start system. Error code: " + std::to_string(err) );
    }
}

Disco2Camera::VimbaProvider::~VimbaProvider(){
    sys.Shutdown();

    // for(CameraPtr cam : cameras){
    //     cam->Close();
    // }
}

void GigEAdjustPacketSize(CameraPtr camera)
{
    StreamPtrVector streams;
    VmbErrorType err = camera->GetStreams(streams);

    if (err != VmbErrorSuccess || streams.empty())
    {
        throw std::runtime_error("Could not get stream modules, err=" + std::to_string(err));
    }

    FeaturePtr feature;
    err = streams[0]->GetFeatureByName("GVSPAdjustPacketSize", feature);

    if (err == VmbErrorSuccess)
    {
        err = feature->RunCommand();
        if (err == VmbErrorSuccess)
        {
            bool commandDone = false;
            do
            {
                if (feature->IsCommandDone(commandDone) != VmbErrorSuccess)
                {
                    break;
                }
            } while (commandDone == false);
        }
        else
        {
            std::cout << "Error while executing GVSPAdjustPacketSize, err=" + std::to_string(err) << std::endl;
        }
    }
}

std::vector<CameraPtr> Disco2Camera::VimbaProvider::GetCameras(){
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

FramePtr Disco2Camera::VimbaProvider::AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain){
    FeaturePtr pFormatFeature;

    // Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
    VmbErrorType err = cam->GetFeatureByName( "PixelFormat", pFormatFeature );
    
    if ( VmbErrorSuccess == err )
    {
        // Try to set BGR
        err = pFormatFeature->SetValue( VmbPixelFormatRgb8 );

        // Try to set exposure time: GigE: ExposureTimeAbs; Alvium USB3: ExposureTime
        FeaturePtr pExposureFeature, pGainFeature;
        
        err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
        if ( VmbErrorSuccess != err ) { // try USB3 feature name
            err = cam->GetFeatureByName("ExposureTime", pExposureFeature);
        }

        if (VmbErrorSuccess == err) {
            std::cout << exposure << std::endl;
            err = pExposureFeature->SetValue(exposure); // in us, 15000.0us = 15ms
        }

        err = cam->GetFeatureByName("Gain", pGainFeature);

        if (VmbErrorSuccess == err) {
            std::cout << gain << std::endl;
            err = pGainFeature->SetValue(gain); // in us, 15000.0us = 15ms
        }
    }

    FramePtr frame;
    
    err = cam->AcquireSingleImage(frame, 5000);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    return frame;
}