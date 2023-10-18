#include <VmbCPP/VmbCPP.h>
#include <vector>

#ifndef VIMBAPROVIDER_H
#define VIMBAPROVIDER_H

namespace Disco2Camera{
    class VimbaProvider{
        private:
            VmbCPP::VmbSystem& sys;
            std::vector<VmbCPP::CameraPtr> cameras;

        public:
            VimbaProvider();
            ~VimbaProvider();

            std::vector<VmbCPP::CameraPtr> GetCameras();

            VmbCPP::FramePtr AqcuireFrame(VmbCPP::CameraPtr cam);
            VmbCPP::FramePtr AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain);
            
            void SetPixleFormat(VmbCPP::CameraPtr cam);
            void SetExposure(VmbCPP::CameraPtr cam);
            void SetGain(VmbCPP::CameraPtr cam);
    };
}

#endif