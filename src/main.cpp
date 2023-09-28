#include <iostream>
#include "include/VimbaProvider.hpp"

using namespace Disco2Camera;

int main(){
    VimbaProvider* p = new VimbaProvider();

    p->GetCameras();

    delete p;
    return 0;
}