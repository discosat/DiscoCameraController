#ifndef VIMBA_CONTROLLER_H
#define VIMBA_CONTROLLER_H

#include <VmbCPP/VmbCPP.h>
#include <vector>
#include "camera_controller.hpp"

#define VMB_BPP 12 // Bits per pixel in the raw output of BayerRG

class VimbaController: public CameraController {
    private:
        VmbCPP::VmbSystem& sys;
        std::vector<VmbCPP::CameraPtr> cameras;

        std::vector<VmbCPP::CameraPtr> GetCameras();
        VmbCPP::FramePtrVector AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain, int numFrames);

    public:
        VimbaController();
        ~VimbaController() override;

        std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) override;
};

#endif