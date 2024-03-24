#include <VmbCPP/VmbCPP.h>
#include <vector>

#ifndef VIMBA_CONTROLLER_H
#define VIMBA_CONTROLLER_H

class VimbaController{
    private:
        VmbCPP::VmbSystem& sys;
        std::vector<VmbCPP::CameraPtr> cameras;

    public:
        VimbaController();
        ~VimbaController();

        std::vector<VmbCPP::CameraPtr> GetCameras();

        VmbCPP::FramePtr AqcuireFrame(VmbCPP::CameraPtr cam);
        VmbCPP::FramePtrVector AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain, int numFrames);
        
        void SetPixleFormat(VmbCPP::CameraPtr cam);
        void SetExposure(VmbCPP::CameraPtr cam);
        void SetGain(VmbCPP::CameraPtr cam);

        double GetFeature(std::string feature_name, VmbCPP::CameraPtr camera);
};

#endif