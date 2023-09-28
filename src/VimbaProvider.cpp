#include <iostream>
#include "include/VimbaProvider.hpp" 
#include <VmbCPP/VmbCPP.h>

using namespace VmbCPP;

Disco2Camera::VimbaProvider::VimbaProvider(){}

Disco2Camera::VimbaProvider::~VimbaProvider(){}

void Disco2Camera::VimbaProvider::GetCameras(){
    VmbSystem& sys = VmbSystem::GetInstance();  // Get a reference to the VimbaSystem singleton

    VmbVersionInfo_t versionInfo;
    sys.QueryVersion(versionInfo);
    std::cout << "Vmb Version Major: " << versionInfo.major << " Minor: " << versionInfo.minor << " Patch: " << versionInfo.patch << "\n\n";

    VmbErrorType err = sys.Startup();           // Initialize the Vmb API
    if (VmbErrorSuccess == err)
    {

        TransportLayerPtrVector transportlayers;             // A vector of std::shared_ptr<AVT::VmbAPI::TransportLayer> objects
        err = sys.GetTransportLayers(transportlayers);       // Fetch all transport layers
        if (VmbErrorSuccess == err) std::cout << "TransportLayers found: " << transportlayers.size() << "\n";

        InterfacePtrVector interfaces;             // A vector of std::shared_ptr<AVT::VmbAPI::Interface> objects
        err = sys.GetInterfaces(interfaces);       // Fetch all interfaces
        if (VmbErrorSuccess == err) std::cout << "Interfaces found: " << interfaces.size() << "\n";

        CameraPtrVector cameras;                // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
        err = sys.GetCameras(cameras);          // Fetch all cameras
        if (VmbErrorSuccess == err)
        {
            std::cout << "Cameras found: " << cameras.size() << "\n\n";

            // Query all static details of all known cameras and print them out.
            // We don't have to open the cameras for that.
            for(const CameraPtr& cam : cameras){
                std::cout << "asdf";
            }
        }
        else
        {
            std::cout << "Could not list cameras. Error code: " << err << "\n";
        }

        sys.Shutdown();
    }
    else
    {
        std::cout << "Could not start system. Error code: " << err << "\n";
    }
}