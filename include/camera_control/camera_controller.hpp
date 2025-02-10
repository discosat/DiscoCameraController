#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <iostream>
#include <memory>
#include <vector>
#include <common.hpp>
#include <map>

const std::map<int, CameraType> CameraTypeMap = {
    {0, CameraType::VMB},
    {1, CameraType::IR},
    {2, CameraType::TEST},
};

static inline CameraType IntToCameraType(int type) {
    if (CameraTypeMap.count(type)) {
        return CameraTypeMap.at(type);
    } else {
        return CameraType::Unknown;
    }
}

class CameraController {
public:
    CameraController(){}
    virtual ~CameraController() = default;
    
    virtual std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) = 0;
};

#endif