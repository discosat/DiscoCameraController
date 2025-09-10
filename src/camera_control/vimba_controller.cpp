#include "vimba_controller.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <VmbCPP/VmbCPP.h>
#include "errors.hpp"
#include "common.hpp"
#include <memory>
#include <cstring>
#include <fstream>
#include "param_config.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

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

int VimbaController::getBitsPerPixelFromFormat(VmbPixelFormatType format) {
    switch(format) {
        case VmbPixelFormatBayerGR8:
        case VmbPixelFormatBayerRG8:
        case VmbPixelFormatBayerGB8:
        case VmbPixelFormatBayerBG8:
        case VmbPixelFormatMono8:
        case VmbPixelFormatRgb8:
            return 8;
            
        case VmbPixelFormatBayerGR10:
        case VmbPixelFormatBayerRG10:
        case VmbPixelFormatBayerGB10:
        case VmbPixelFormatBayerBG10:
        case VmbPixelFormatMono10:
        case VmbPixelFormatRgb10:
            return 10;
            
        case VmbPixelFormatBayerGR12:
        case VmbPixelFormatBayerRG12:
        case VmbPixelFormatBayerGB12:
        case VmbPixelFormatBayerBG12:
        case VmbPixelFormatMono12:
        case VmbPixelFormatRgb12:
            return 12;
            
        case VmbPixelFormatBayerGR16:
        case VmbPixelFormatBayerRG16:
        case VmbPixelFormatBayerGB16:
        case VmbPixelFormatBayerBG16:
        case VmbPixelFormatMono16:
        case VmbPixelFormatRgb16:
            return 16;
            
        default:
            std::cerr << "Unknown pixel format, defaulting to 8 bits" << std::endl;
            return 8;
    }
}

int VimbaController::getChannelsFromFormat(VmbPixelFormatType format) {
    switch(format) {
        case VmbPixelFormatRgb8:
        case VmbPixelFormatRgb10:
        case VmbPixelFormatRgb12:
        case VmbPixelFormatRgb16:
            return 3;  // RGB has 3 channels
            
        case VmbPixelFormatBayerGR8:
        case VmbPixelFormatBayerRG8:
        case VmbPixelFormatBayerGB8:
        case VmbPixelFormatBayerBG8:
        case VmbPixelFormatBayerGR10:
        case VmbPixelFormatBayerRG10:
        case VmbPixelFormatBayerGB10:
        case VmbPixelFormatBayerBG10:
        case VmbPixelFormatBayerGR12:
        case VmbPixelFormatBayerRG12:
        case VmbPixelFormatBayerGB12:
        case VmbPixelFormatBayerBG12:
        case VmbPixelFormatBayerGR16:
        case VmbPixelFormatBayerRG16:
        case VmbPixelFormatBayerGB16:
        case VmbPixelFormatBayerBG16:
        case VmbPixelFormatMono8:
        case VmbPixelFormatMono10:
        case VmbPixelFormatMono12:
        case VmbPixelFormatMono16:
            return 1;  // Bayer and Mono have 1 channel
            
        default:
            std::cerr << "Unknown pixel format, defaulting to 1 channel" << std::endl;
            return 1;
    }
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

bool VimbaController::saveImageAsPNG(u_char* buffer, u_int width, u_int height, int bitsPerPixel, VmbPixelFormatType pixelFormat, const std::string& filename) {
    try {
        cv::Mat image;
        
        if (pixelFormat == VmbPixelFormatRgb8) {
            // RGB8 format - convert to grayscale
            cv::Mat rgb_image(height, width, CV_8UC3, buffer);
            cv::cvtColor(rgb_image, image, cv::COLOR_RGB2GRAY);
        }
        else if (pixelFormat == VmbPixelFormatMono8) {
            // Mono8 format - direct grayscale
            image = cv::Mat(height, width, CV_8UC1, buffer);
        }
        else if (pixelFormat == VmbPixelFormatMono12 || 
                 pixelFormat == VmbPixelFormatBayerGR12 || 
                 pixelFormat == VmbPixelFormatBayerRG12 ||
                 pixelFormat == VmbPixelFormatBayerGB12 || 
                 pixelFormat == VmbPixelFormatBayerBG12) {
            // 12-bit formats - typically stored in 16-bit containers
            // Most cameras store 12-bit data in 16-bit containers with the 12 bits left-aligned or right-aligned
            
            // Try interpreting as 16-bit data first (most common case)
            cv::Mat temp(height, width, CV_16UC1, buffer);
            
            // Check if data looks like 12-bit in 16-bit containers
            uint16_t* data16 = reinterpret_cast<uint16_t*>(buffer);
            uint16_t maxVal = 0;
            for (size_t i = 0; i < std::min((size_t)1000, (size_t)(width * height)); i++) {
                maxVal = std::max(maxVal, data16[i]);
            }
            
            if (maxVal > 4095) {
                // Data appears to be left-aligned 12-bit (shifted left by 4)
                // Scale from 16-bit range to 8-bit
                temp.convertTo(image, CV_8UC1, 1.0/256.0);
                std::cout << "Treating as left-aligned 12-bit data (max sample: " << maxVal << ")" << std::endl;
            } else {
                // Data appears to be right-aligned 12-bit (0-4095 range)
                // Scale from 12-bit range to 8-bit
                temp.convertTo(image, CV_8UC1, 1.0/16.0);
                std::cout << "Treating as right-aligned 12-bit data (max sample: " << maxVal << ")" << std::endl;
            }
        }
        else if (pixelFormat == VmbPixelFormatMono16 ||
                 pixelFormat == VmbPixelFormatBayerGR16 || 
                 pixelFormat == VmbPixelFormatBayerRG16 ||
                 pixelFormat == VmbPixelFormatBayerGB16 || 
                 pixelFormat == VmbPixelFormatBayerBG16) {
            // 16-bit formats - scale down to 8-bit
            cv::Mat temp(height, width, CV_16UC1, buffer);
            temp.convertTo(image, CV_8UC1, 1.0/256.0);
        }
        else {
            // Fallback: assume 8-bit mono
            std::cout << "Warning: Unknown pixel format, treating as 8-bit mono" << std::endl;
            image = cv::Mat(height, width, CV_8UC1, buffer);
        }
        
        // Save as PNG
        bool success = cv::imwrite(filename, image);
        if (success) {
            std::cout << "Image saved as " << filename << " (" << image.cols << "x" << image.rows << ")" << std::endl;
        } else {
            std::cerr << "Failed to save image as " << filename << std::endl;
        }
        return success;
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error saving image: " << e.what() << std::endl;
        return false;
    }
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

FramePtr VimbaController::aqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain){
    if(!turnOnCamera(cam)){
        throw std::runtime_error("Could not turn on camera");
    }

    // First, ensure the camera is ready for acquisition
    FeaturePtr pAcquisitionMode;
    VmbErrorType err = cam->GetFeatureByName("AcquisitionMode", pAcquisitionMode);
    if (err == VmbErrorSuccess) {
        err = pAcquisitionMode->SetValue("SingleFrame");
        if (err != VmbErrorSuccess) {
            std::cerr << "Warning: Could not set AcquisitionMode to SingleFrame, err=" << err << std::endl;
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
            // Use full width for complete image capture
            VmbInt64_t fullWidth = maxWidth;
            // Ensure proper alignment based on pixel format
            // For 12-bit packed: width should be even (2 pixels per 3 bytes)
            fullWidth = (fullWidth / 2) * 2;  // Even alignment for 12-bit packed
            pWidth->SetValue(fullWidth);
            std::cout << "Set width to " << fullWidth << " (full sensor width)" << std::endl;
        }
    }
    if (cam->GetFeatureByName("Height", pHeight) == VmbErrorSuccess) {
        if (pHeight->GetRange(minHeight, maxHeight) == VmbErrorSuccess) {
            // Use full height for complete image capture
            VmbInt64_t fullHeight = maxHeight;
            // Ensure height is multiple of 2 for proper alignment
            fullHeight = (fullHeight / 2) * 2;
            pHeight->SetValue(fullHeight);
            std::cout << "Set height to " << fullHeight << " (full sensor height)" << std::endl;
        }
    }
    
    // Configure USB specific settings for reliable data transfer
    FeaturePtr pDeviceLinkThroughputLimit;
    err = cam->GetFeatureByName("DeviceLinkThroughputLimit", pDeviceLinkThroughputLimit);
    if (err == VmbErrorSuccess) {
        // Very conservative bandwidth for USB2 to ensure complete transfer
        VmbInt64_t limitValue = 20000000; // 20 MB/s for USB2 compatibility
        err = pDeviceLinkThroughputLimit->SetValue(limitValue);
        if (err == VmbErrorSuccess) {
            std::cout << "Set USB bandwidth limit to " << limitValue << " bytes/sec" << std::endl;
        }
    }
    
    // Set packet size for USB2 compatibility
    FeaturePtr pPacketSize;
    err = cam->GetFeatureByName("GVSPPacketSize", pPacketSize);
    if (err != VmbErrorSuccess) {
        // Try alternative packet size feature name for USB cameras
        err = cam->GetFeatureByName("PacketSize", pPacketSize);
    }
    if (err == VmbErrorSuccess) {
        VmbInt64_t minPacket, maxPacket;
        if (pPacketSize->GetRange(minPacket, maxPacket) == VmbErrorSuccess) {
            // Use smaller, safer packet size for USB2
            VmbInt64_t usbPacketSize = std::min((VmbInt64_t)512, maxPacket);
            pPacketSize->SetValue(usbPacketSize);
            std::cout << "Set packet size to " << usbPacketSize << " bytes for USB2" << std::endl;
        }
    }
    
    // Set stream buffer count for USB2
    FeaturePtr pStreamBufferCount;
    err = cam->GetFeatureByName("StreamBufferCount", pStreamBufferCount);
    if (err == VmbErrorSuccess) {
        pStreamBufferCount->SetValue(10); // More buffers for USB2 reliability
        std::cout << "Set stream buffer count to 10 for USB2" << std::endl;
    }
    
    FeaturePtr pFormatFeature;
    err = cam->GetFeatureByName( "PixelFormat", pFormatFeature );
    
    if ( VmbErrorSuccess == err )
    {
        err = pFormatFeature->SetValue( VmbPixelFormatBayerGR12 );
        if ( VmbErrorSuccess != err ) {
            err = pFormatFeature->SetValue( VmbPixelFormatBayerRG12 );
            if ( VmbErrorSuccess != err ) {
                pFormatFeature->SetValue( VmbPixelFormatRgb8 );
            }
        }
        
        FeaturePtr pExposureFeature, pGainFeature;
        
        err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
        if ( VmbErrorSuccess != err ) {
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
    // Increase timeout significantly for USB2 to allow for slower data transfer
    err = cam->AcquireSingleImage(frame, 30000); // 30 seconds timeout for USB2

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

    if(!turnOffCamera(cam)){
        std::cerr << "Could not turn off camera" << std::endl;
    }

    return frame;
}

std::vector<Image> VimbaController::Capture(CaptureMessage& capture_instructions, u_int16_t* error) {
    std::vector<VmbCPP::CameraPtr> cameras = this->getCameras();
    VmbCPP::CameraPtr cam;
    std::vector<Image> images;
    
    if(cameras.size() > 0){
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);
            std::cout << "Camera found: " << camName << std::endl;

            if(camName == capture_instructions.CameraId){
                cam = cameras.at(i);
            }
        }
        
        // If no matching camera found but cameras exist, use the first one
        if(cam == NULL && cameras.size() > 0){
            cam = cameras.at(0);
            std::string camName;
            cam->GetModel(camName);
            std::cout << "Warning: Camera '" << capture_instructions.CameraId << "' not found. Using first available camera: " << camName << std::endl;
        }
    } else {
        *error = ERROR_CODE::CAPTURE_ERROR_NO_CAMERAS;
        std::cout << "No cameras found!" << std::endl;
        return images;
    }

    if(cam != NULL){
        VmbErrorType openErr = cam->Open(VmbAccessModeExclusive);
        if (openErr != VmbErrorSuccess) {
            std::cerr << "Failed to open camera. Error code: " << openErr << std::endl;
            *error = ERROR_CODE::CAPTURE_ERROR;
            return images;
        }
        
        for(size_t i = 0; i < capture_instructions.NumberOfImages; i++){
            VmbCPP::FramePtr frame;
            size_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            try{
		    
                frame = this->aqcuireFrame(cam, capture_instructions.Exposure, capture_instructions.ISO);
            } catch(const std::exception& e){
                std::cout << "Error!" << std::endl;
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
            int bitsPerPixel = getBitsPerPixelFromFormat(pixelFormat);
            int channels = getChannelsFromFormat(pixelFormat);
            
            

            Image img;
            img.size = bufferSize;
            img.width = width;
            img.height = height;
            img.data = new u_char[bufferSize];
            img.bpp = bitsPerPixel;
            img.channels = channels;
            img.timestamp = timestamp;
            
            std::cout << "Captured image with pixel format: 0x" << std::hex << pixelFormat << std::dec
                      << ", bits per pixel: " << bitsPerPixel 
                      << ", channels: " << channels
                      << ", width: " << width 
                      << ", height: " << height 
                      << ", buffer size: " << bufferSize << " bytes" << std::endl;
            
            // Copy the actual image data from buffer to img.data
            std::memcpy(img.data, buffer, bufferSize);

            // Save image as grayscale PNG
            std::string pngFilename = "captured_image_" + std::to_string(timestamp) + ".png";
            saveImageAsPNG(buffer, width, height, bitsPerPixel, pixelFormat, pngFilename);
            //std::memcpy(img.data, &buffer, bufferSize * sizeof(u_char));

            images.push_back(img);

            // delay if needed
            if(i < capture_instructions.NumberOfImages - 1 && capture_instructions.Interval > 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(capture_instructions.Interval));
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
