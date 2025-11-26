#include "vimba_controller.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <VmbCPP/VmbCPP.h>
#include "errors.hpp"
#include "common.hpp"
#include "logger.hpp"
#include <memory>
#include <cstring>
#include <fstream>
#include "param_config.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>

extern "C" {
#include "gpio_control.h"
}

using namespace VmbCPP;

VimbaController::VimbaController() : sys(VmbSystem::GetInstance()){
    VmbErrorType err = this->sys.Startup();

    if (VmbErrorSuccess != err)
    {
        throw std::runtime_error("Could not start system. Error code: " + std::to_string(err) );
    }
}

VimbaController::~VimbaController(){
    sys.Shutdown();
}

bool VimbaController::readCameraTemperature(VmbCPP::CameraPtr cam, double& temperature) {
    if (cam == nullptr) {
        return false;
    }
    
    // Try common temperature feature names used by different camera manufacturers
    std::vector<std::string> temperatureFeatureNames = {
        "DeviceTemperature",
        "DeviceTemperatureSelector",
        "SensorTemperature", 
        "CameraTemperature",
        "Temperature"
    };
    
    for (const auto& featureName : temperatureFeatureNames) {
        FeaturePtr tempFeature;
        VmbErrorType err = cam->GetFeatureByName(featureName.c_str(), tempFeature);
        
        if (err == VmbErrorSuccess) {
            // Try to read as double first
            double tempValue;
            err = tempFeature->GetValue(tempValue);
            if (err == VmbErrorSuccess) {
                temperature = tempValue;
                return true;
            }
            
            // If double fails, try as int64
            VmbInt64_t tempValueInt;
            err = tempFeature->GetValue(tempValueInt);
            if (err == VmbErrorSuccess) {
                temperature = static_cast<double>(tempValueInt);
                return true;
            }
        }
    }
    
    return false;
}

std::vector<CameraPtr> VimbaController::getCameras(){
    CameraPtrVector cams;
    VmbErrorType err = sys.GetCameras(cams);

    if (VmbErrorSuccess == err)
    {
        return cams;
    }
    else
    {
        sys.Shutdown();
        throw std::runtime_error("Could not list cameras. Error code: " + std::to_string(err) );
    }
}

bool VimbaController::turnOnCamera(VmbCPP::CameraPtr cam){
    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName( "DevicePowerSavingMode", powerSavingFeature );

    if ( VmbErrorSuccess == err ){
        err = powerSavingFeature->SetValue("Disabled");
        return VmbErrorSuccess == err;
    } else {
        return false;
    }
}

bool VimbaController::turnOffCamera(VmbCPP::CameraPtr cam){
    // First try to ensure acquisition is stopped
    FeaturePtr pAcquisitionStop;
    VmbErrorType stopErr = cam->GetFeatureByName("AcquisitionStop", pAcquisitionStop);
    if (stopErr == VmbErrorSuccess) {
        pAcquisitionStop->RunCommand();
    }

    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName( "DevicePowerSavingMode", powerSavingFeature );

    if ( VmbErrorSuccess == err ){
        err = powerSavingFeature->SetValue("SuspendMode");
        return VmbErrorSuccess == err;
    } else {
        return false;
    }
}

bool VimbaController::suspendCamera(VmbCPP::CameraPtr cam){
    // Stop any ongoing acquisition
    FeaturePtr pAcquisitionStop;
    VmbErrorType stopErr = cam->GetFeatureByName("AcquisitionStop", pAcquisitionStop);
    if (stopErr == VmbErrorSuccess) {
        pAcquisitionStop->RunCommand();
    }

    // Put camera into suspend mode (low power state, fast wake-up)
    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName("DevicePowerSavingMode", powerSavingFeature);

    if (VmbErrorSuccess == err) {
        err = powerSavingFeature->SetValue("SuspendMode");
        if (VmbErrorSuccess == err) {
            DiscoLogger::camera("Camera suspended (DevicePowerSavingMode=SuspendMode)");
            return true;
        }
    }
    DiscoLogger::warning("Failed to suspend camera");
    return false;
}

bool VimbaController::wakeCamera(VmbCPP::CameraPtr cam){
    // Wake camera from suspend mode
    FeaturePtr powerSavingFeature;
    VmbErrorType err = cam->GetFeatureByName("DevicePowerSavingMode", powerSavingFeature);

    if (VmbErrorSuccess == err) {
        err = powerSavingFeature->SetValue("Disabled");
        if (VmbErrorSuccess == err) {
            DiscoLogger::camera("Camera woken (DevicePowerSavingMode=Disabled)");
            return true;
        }
    }
    DiscoLogger::warning("Failed to wake camera");
    return false;
}

FramePtr VimbaController::aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain){
    // Camera power is now controlled via GPIO hardware, not software suspend

    // Set acquisition mode to single frame
    FeaturePtr pAcquisitionMode;
    VmbErrorType err = cam->GetFeatureByName("AcquisitionMode", pAcquisitionMode);
    if (err == VmbErrorSuccess) {
        err = pAcquisitionMode->SetValue("SingleFrame");
        if (err != VmbErrorSuccess) {
            std::stringstream ss;
            ss << "Could not set AcquisitionMode to SingleFrame (error=" << err << ")";
            DiscoLogger::warning(ss.str());
        }
    }

    // Configure camera for full sensor capture
    // Reset any existing ROI and use maximum available resolution
    FeaturePtr pOffsetX, pOffsetY, pWidth, pHeight;

    // Reset ROI offsets to 0 (top-left corner)
    if (cam->GetFeatureByName("OffsetX", pOffsetX) == VmbErrorSuccess) {
        pOffsetX->SetValue(0);
    }
    if (cam->GetFeatureByName("OffsetY", pOffsetY) == VmbErrorSuccess) {
        pOffsetY->SetValue(0);
    }

    // Set to maximum sensor resolution for full image capture
    VmbInt64_t minWidth, maxWidth, minHeight, maxHeight;
    if (cam->GetFeatureByName("Width", pWidth) == VmbErrorSuccess) {
        if (pWidth->GetRange(minWidth, maxWidth) == VmbErrorSuccess) {
            VmbInt64_t fullWidth = (maxWidth / 2) * 2;  // Ensure even width
            pWidth->SetValue(fullWidth);
            std::stringstream ss;
            ss << "Width set to " << fullWidth << " pixels (full sensor)";
            DiscoLogger::camera(ss.str());
        }
    }
    if (cam->GetFeatureByName("Height", pHeight) == VmbErrorSuccess) {
        if (pHeight->GetRange(minHeight, maxHeight) == VmbErrorSuccess) {
            VmbInt64_t fullHeight = (maxHeight / 2) * 2;  // Ensure even height
            pHeight->SetValue(fullHeight);
            std::stringstream ss;
            ss << "Height set to " << fullHeight << " pixels (full sensor)";
            DiscoLogger::camera(ss.str());
        }
    }

    // Configure USB bandwidth limit
    FeaturePtr pDeviceLinkThroughputLimit;
    err = cam->GetFeatureByName("DeviceLinkThroughputLimit", pDeviceLinkThroughputLimit);
    if (err == VmbErrorSuccess) {
        VmbInt64_t limitValue = 35000000; // 35 MB/s
        err = pDeviceLinkThroughputLimit->SetValue(limitValue);
        if (err == VmbErrorSuccess) {
            DiscoLogger::camera("USB bandwidth limit set to 35 MB/s");
        }
    }

    // Set packet size
    FeaturePtr pPacketSize;
    err = cam->GetFeatureByName("GVSPPacketSize", pPacketSize);
    if (err != VmbErrorSuccess) {
        err = cam->GetFeatureByName("PacketSize", pPacketSize);
    }
    if (err == VmbErrorSuccess) {
        VmbInt64_t minPacket, maxPacket;
        if (pPacketSize->GetRange(minPacket, maxPacket) == VmbErrorSuccess) {
            VmbInt64_t packetSize = std::min((VmbInt64_t)16384, maxPacket);
            packetSize = std::max(packetSize, minPacket);
            pPacketSize->SetValue(packetSize);
            std::stringstream ss;
            ss << "Packet size set to " << packetSize << " bytes";
            DiscoLogger::camera(ss.str());
        }
    }

    // Set stream buffer count
    FeaturePtr pStreamBufferCount;
    err = cam->GetFeatureByName("StreamBufferCount", pStreamBufferCount);
    if (err == VmbErrorSuccess) {
        pStreamBufferCount->SetValue(25);
        DiscoLogger::camera("Stream buffer count set to 25");
    }

    // Set pixel format
    FeaturePtr pFormatFeature;
    err = cam->GetFeatureByName("PixelFormat", pFormatFeature);

    if (VmbErrorSuccess == err)
    {
        err = pFormatFeature->SetValue(VmbPixelFormatBayerRG12);
        if (VmbErrorSuccess != err) {
            DiscoLogger::error("Failed to set pixel format to BayerRG12!");
        }

        // Set exposure and gain
        FeaturePtr pExposureFeature, pGainFeature;

        err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
        if (VmbErrorSuccess != err) {
            err = cam->GetFeatureByName("ExposureTime", pExposureFeature);
        }

        if (VmbErrorSuccess == err) {
            err = pExposureFeature->SetValue(exposure);
        }

        err = cam->GetFeatureByName("Gain", pGainFeature);

        if (VmbErrorSuccess == err) {
            err = pGainFeature->SetValue(gain);
        }
    }

    FramePtr frame;
    err = cam->AcquireSingleImage(frame, 90000); // 90 seconds timeout

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    // Ensure acquisition is stopped after single image capture
    FeaturePtr pAcquisitionStop;
    VmbErrorType stopErr = cam->GetFeatureByName("AcquisitionStop", pAcquisitionStop);
    if (stopErr == VmbErrorSuccess) {
        pAcquisitionStop->RunCommand();
        // Small delay to ensure stop completes
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Camera power-off is now handled via GPIO hardware in the main capture loop

    return frame;
}

std::vector<Image> VimbaController::Capture(CaptureMessage& capture_instructions, u_int16_t* error) {
    std::vector<VmbCPP::CameraPtr> cameras = this->getCameras();
    VmbCPP::CameraPtr cam;
    std::vector<Image> images;
    
    if(cameras.size() > 0){
        // Find the requested camera
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);

            if(camName == capture_instructions.CameraId){
                cam = cameras.at(i);
                break;  // Found it, stop searching
            }
        }

        // Fail gracefully if requested camera not found
        if(!cam){
            *error = ERROR_CODE::CAPTURE_ERROR_CAMERA_NOT_FOUND;
            std::stringstream ss;
            ss << "Camera '" << capture_instructions.CameraId << "' not found!";
            DiscoLogger::error(ss.str());

            DiscoLogger::info("Available cameras:");
            for(size_t i = 0; i < cameras.size(); i++){
                std::string camName;
                cameras.at(i)->GetModel(camName);
                std::stringstream cam_ss;
                cam_ss << "  - " << camName;
                DiscoLogger::info(cam_ss.str());
            }
            return images;
        }
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_NO_CAMERAS;
        DiscoLogger::error("No cameras found!");
        return images;
    }

    if(cam != NULL){
        VmbErrorType openErr = cam->Open(VmbAccessModeExclusive);
        if (openErr != VmbErrorSuccess) {
            std::stringstream ss;
            ss << "Failed to open camera (error code: " << openErr << ")";
            DiscoLogger::error(ss.str());
            *error = ERROR_CODE::CAPTURE_ERROR;
            return images;
        }
        
        for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
            // Check camera state before each capture
            uint8_t camera_state = param_get_uint8(&camera_state_param);
            if (camera_state != 1) {
                DiscoLogger::warning("Camera turned off during batch capture - aborting remaining captures");
                // Clean up any partial images
                for (size_t j = 0; j < images.size(); j++) {
                    delete[] images.at(j).data;
                }
                images.clear();
                *error = ERROR_CODE::CAPTURE_ERROR;

                // Ensure camera is powered off via GPIO
                set_camera_gpio(NULL, 0);
                cam->Close();
                return images;
            }

            VmbCPP::FramePtr frame;
            size_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            try{

                frame = this->aqcuireFrame(cam, capture_instructions.Exposure, capture_instructions.ISO);
            } catch(const std::exception& e){
                DiscoLogger::error("Failed to acquire frame!");
                *error = ERROR_CODE::CAPTURE_ERROR;
                return images;
            }

            u_int width, height, bufferSize;
            frame->GetBufferSize(bufferSize);
            frame->GetWidth(width);
            frame->GetHeight(height);

            u_char* buffer;
            frame->GetImage(buffer);

            // Get the actual pixel format from the frame
            VmbPixelFormatType pixelFormat;
            frame->GetPixelFormat(pixelFormat);

            Image img;
            img.size = bufferSize;
            img.width = width;
            img.height = height;
            img.data = new u_char[bufferSize];
            img.bpp = 12;  // BayerRG12 is always 12-bit
            img.channels = 1;  // Bayer is always 1 channel
            img.timestamp = timestamp;

            std::stringstream ss;
            ss << "Captured image " << (i+1) << "/" << capture_instructions.NumberOfImages
               << " [" << width << "x" << height << ", " << bufferSize << " bytes]";
            DiscoLogger::success(ss.str());
            
            // Copy the actual image data from buffer to img.data
            std::memcpy(img.data, buffer, bufferSize);

            images.push_back(img);

            // Suspend camera between captures (if more images remain)
            if(i < capture_instructions.NumberOfImages - 1){
                DiscoLogger::camera("Suspending camera between captures...");

                // Suspend camera (keeps GPIO power on, uses DevicePowerSavingMode)
                this->suspendCamera(cam);

                // Set camera state param to suspend (2)
                param_set_uint8(&camera_state_param, 2);

                // Apply interval delay (if specified)
                if(capture_instructions.Interval > 0){
                    std::stringstream interval_ss;
                    interval_ss << "Waiting " << capture_instructions.Interval << "ms interval...";
                    DiscoLogger::camera(interval_ss.str());
                    std::this_thread::sleep_for(std::chrono::milliseconds(capture_instructions.Interval));
                }

                // Wake camera from suspend
                this->wakeCamera(cam);

                // Set camera state param back to on (1)
                param_set_uint8(&camera_state_param, 1);

                // Small delay to allow camera to stabilize after wake
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                std::stringstream success_ss;
                success_ss << "Camera ready for capture " << (i+2)
                          << "/" << capture_instructions.NumberOfImages;
                DiscoLogger::success(success_ss.str());
            }
        }

        
        // Ensure any acquisition is stopped before closing
        // Try to stop acquisition if it's running
        FeaturePtr pAcquisitionStop;
        VmbErrorType stopErr = cam->GetFeatureByName("AcquisitionStop", pAcquisitionStop);
        if (stopErr == VmbErrorSuccess) {
            VmbErrorType cmdErr = pAcquisitionStop->RunCommand();
            if (cmdErr != VmbErrorSuccess) {
                std::cerr << "Warning: AcquisitionStop command failed with error: " << cmdErr << std::endl;
            }
            // Give it a moment to complete
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
        }
        
        // Try to flush the queue if the camera has one
        cam->FlushQueue();
        
        // Add a delay to ensure all operations are complete
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
 	VmbErrorType closeErr = cam->Close();
	if (closeErr != VmbErrorSuccess) {
	    std::cerr << "Failed to close camera. Error code: " << closeErr << std::endl;
	    std::cerr << "Error details: ";
	    switch(closeErr) {
	        case VmbErrorDeviceNotOpen:
	            std::cerr << "Camera was not opened" << std::endl;
	            break;
	        case VmbErrorInUse:
	            std::cerr << "Camera is currently in use" << std::endl;
	            break;
	        case VmbErrorBadHandle:
	            std::cerr << "Invalid camera handle" << std::endl;
	            break;
	        case VmbErrorInvalidCall:
	            std::cerr << "Invalid call (possibly from callback)" << std::endl;
	            break;
	        default:
	            std::cerr << "Unknown error" << std::endl;
	    }
	    *error = ERROR_CODE::CAPTURE_ERROR;
	} else {
	}

    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_CAMERA_NOT_FOUND;
    }

    return images;
}

bool VimbaController::updateTemperatureParameter() {
    std::vector<VmbCPP::CameraPtr> cameras = this->getCameras();
    
    if (cameras.empty()) {
        std::cerr << "No cameras available for temperature reading" << std::endl;
        return false;
    }
    
    // Use the first available camera for temperature reading
    VmbCPP::CameraPtr cam = cameras[0];
    
    VmbErrorType openErr = cam->Open(VmbAccessModeExclusive);
    if (openErr != VmbErrorSuccess) {
        std::cerr << "Failed to open camera for temperature reading. Error code: " << openErr << std::endl;
        return false;
    }
    
    double temperature;
    bool success = false;
    
    if (readCameraTemperature(cam, temperature)) {
        param_set_double(&camera_temperature_param, temperature);
        std::cout << "Camera temperature updated: " << temperature << "°C" << std::endl;
        success = true;
    } else {
        std::cout << "Warning: Could not read camera temperature" << std::endl;
    }
    
    // Close the camera
    cam->Close();
    
    return success;
}
