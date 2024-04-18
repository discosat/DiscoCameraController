#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <iostream>
#include <memory>
#include <vector>
#include <common.hpp>
#include <map>

const std::map<std::string, CameraType> CameraTypeMap = {
    {"VMB", CameraType::VMB},
    {"IR", CameraType::IR},
    {"TEST", CameraType::TEST},
};

static inline CameraType StringToCameraType(std::string type) {
    if (CameraTypeMap.count(type)) {
        return CameraTypeMap.at(type);
    } else {
        return CameraType::Unkown;
    }
}

class CameraController {
public:
    CameraController(){}
    virtual ~CameraController() = default;
    
    virtual std::vector<Image> Capture(CaptureMessage& capture_instructions, u_int16_t* error) = 0;
};

#endif