#ifndef VIMBA_CONTROLLER_H
#define VIMBA_CONTROLLER_H

#include <VmbCPP/VmbCPP.h>
#include <vector>
#include "common.hpp"
#include "camera_controller.hpp"

// Channels will be determined dynamically based on pixel format

class VimbaController: public CameraController {
    private:
        VmbCPP::VmbSystem& sys;
        std::vector<VmbCPP::CameraPtr> cameras;

        // Camera configuration structure
        struct CameraConfig {
            std::string modelName;
            VmbPixelFormatType preferredPixelFormat;
            VmbInt64_t usbBandwidthLimit;
            VmbInt64_t packetSize;
            int streamBufferCount;
            bool useFullResolution;
            int acquisitionTimeout;
        };

        std::vector<VmbCPP::CameraPtr> getCameras();
        VmbCPP::FramePtr aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain);
        bool turnOnCamera(VmbCPP::CameraPtr cam);
        bool turnOffCamera(VmbCPP::CameraPtr cam);
        int getBitsPerPixelFromFormat(VmbPixelFormatType format);
        int getChannelsFromFormat(VmbPixelFormatType format);
        bool readCameraTemperature(VmbCPP::CameraPtr cam, double& temperature);
        bool saveImageAsTIFF(u_char* buffer, u_int width, u_int height, int bitsPerPixel, VmbPixelFormatType pixelFormat, const std::string& filename);
        CameraConfig getCameraConfig(const std::string& cameraModel);
        void applyCameraConfig(VmbCPP::CameraPtr cam, const CameraConfig& config);

    public:
        VimbaController();
        ~VimbaController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
        bool updateTemperatureParameter();
};

#endif
