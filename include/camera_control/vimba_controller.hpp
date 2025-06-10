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

        std::vector<VmbCPP::CameraPtr> getCameras();
        VmbCPP::FramePtr aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain);
        bool turnOnCamera(VmbCPP::CameraPtr cam);
        bool turnOffCamera(VmbCPP::CameraPtr cam);
        int getBitsPerPixelFromFormat(VmbPixelFormatType format);
        int getChannelsFromFormat(VmbPixelFormatType format);

    public:
        VimbaController();
        ~VimbaController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
};

#endif
