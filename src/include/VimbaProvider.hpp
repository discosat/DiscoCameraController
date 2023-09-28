#include <VmbCPP/VmbCPP.h>

#ifndef VIMBAPROVIDER_H
#define VIMBAPROVIDER_H

namespace Disco2Camera{
    class VimbaProvider{
        public:
            VimbaProvider();
            ~VimbaProvider();

            void GetCameras();
    };
}

#endif