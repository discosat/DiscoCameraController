#include <iostream>
#include <stdexcept>
#include <VmbCPP/VmbCPP.h>
#include "include/VimbaProvider.hpp" 

using namespace VmbCPP;

Disco2Camera::VimbaProvider::VimbaProvider() : sys(VmbSystem::GetInstance()){
    VmbErrorType err = sys.Startup();

    if (VmbErrorSuccess == err)
    {
        throw std::runtime_error("Could not start system. Error code: " + std::to_string(err) );
    }
}

Disco2Camera::VimbaProvider::~VimbaProvider(){
    sys.Shutdown();
}

std::vector<CameraPtr> Disco2Camera::VimbaProvider::GetCameras(){
    TransportLayerPtrVector transportlayers;
    VmbErrorType err = sys.GetTransportLayers(transportlayers);

    InterfacePtrVector interfaces;
    err = sys.GetInterfaces(interfaces);

    CameraPtrVector cameras;
    err = sys.GetCameras(cameras);

    if (VmbErrorSuccess == err)
    {
        return cameras;
    }
    else
    {
        sys.Shutdown();
        throw std::runtime_error("Could not list cameras. Error code: " + std::to_string(err) );
    }
}

FramePtr Disco2Camera::VimbaProvider::AqcuireFrame(CameraPtr cam){
    VmbErrorType err = cam->Open(VmbAccessModeFull);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not open camera, err=" + std::to_string(err));
    }

    FramePtr frame;
    
    err = cam->AcquireSingleImage(frame, 5000);
    
    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    FeaturePtrVector features;
    cam->GetFeatures(features);

    for(FeaturePtr f : features){
        std::string desc;
        std::string cat;
        std::string value;
        
        f->GetDescription(desc);
        f->GetCategory(cat);
        f->GetValue(value);

        std::cout << desc << " " << cat << " " << value << std::endl;
    }

    cam->Close();

    return frame;
}