#include <VmbCPP/VmbCPP.h>
#include <vector>

#ifndef VIMBAPROVIDER_H
#define VIMBAPROVIDER_H

namespace Disco2Camera{
    class VimbaProvider{
        private:
            VmbCPP::VmbSystem& sys;

            //void AdjustPacketSize(CameraPtr camera);

        public:
            VimbaProvider();
            ~VimbaProvider();

            std::vector<VmbCPP::CameraPtr> GetCameras();
            VmbCPP::FramePtr AqcuireFrame(VmbCPP::CameraPtr cam);
    };
}

#endif