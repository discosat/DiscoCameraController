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

VimbaController::CameraConfig VimbaController::getCameraConfig(const std::string& cameraModel) {
    CameraConfig config;
    config.modelName = cameraModel;

    // Default configuration (conservative, works for most cameras)
    config.preferredPixelFormat = VmbPixelFormatBayerGR12;
    config.usbBandwidthLimit = 25000000;  // 25 MB/s
    config.packetSize = 4096;
    config.streamBufferCount = 10;
    config.useFullResolution = true;
    config.acquisitionTimeout = 30000;  // 30 seconds

    // Camera-specific optimizations
    if (cameraModel.find("1800 U-500") != std::string::npos) {
        // 1800 U-500c: 5MP camera, needs conservative USB settings
        std::cout << "Configuring for Allied Vision 1800 U-500 (5MP)" << std::endl;
        config.preferredPixelFormat = VmbPixelFormatBayerGR12;  // Use 12-bit
        config.usbBandwidthLimit = 30000000;  // 30 MB/s
        config.packetSize = 8192;
        config.streamBufferCount = 20;
        config.acquisitionTimeout = 60000;  // 60 seconds for large images
    }
    else if (cameraModel.find("1800 U-507") != std::string::npos) {
        // 1800 U-507c: 5MP camera, similar to U-500 but slightly different sensor
        std::cout << "Configuring for Allied Vision 1800 U-507 (5MP)" << std::endl;
        config.preferredPixelFormat = VmbPixelFormatBayerGR12;  // Use 12-bit
        config.usbBandwidthLimit = 32000000;  // 32 MB/s
        config.packetSize = 8192;
        config.streamBufferCount = 20;
        config.acquisitionTimeout = 60000;
    }
    else if (cameraModel.find("1800 U-811") != std::string::npos) {
        // 1800 U-811c: 12MP camera, highest resolution, needs most conservative settings
        std::cout << "Configuring for Allied Vision 1800 U-811 (12MP)" << std::endl;
        config.preferredPixelFormat = VmbPixelFormatBayerGR12;  // Use 12-bit for 12MP
        config.usbBandwidthLimit = 35000000;  // 35 MB/s (near USB2 practical limit)
        config.packetSize = 16384;  // Larger packets for efficiency
        config.streamBufferCount = 25;  // More buffers for large images
        config.acquisitionTimeout = 90000;  // 90 seconds for very large images
    }
    else if (cameraModel.find("IR") != std::string::npos) {
        // IR camera configuration - assuming it's also USB2
        std::cout << "Configuring for IR camera" << std::endl;
        config.preferredPixelFormat = VmbPixelFormatMono12;  // IR cameras typically use mono, 12-bit
        config.usbBandwidthLimit = 30000000;  // 30 MB/s
        config.packetSize = 8192;
        config.streamBufferCount = 15;
        config.acquisitionTimeout = 45000;
    }
    else if (cameraModel.find("1800 U-") != std::string::npos) {
        // Generic 1800 U-series configuration
        std::cout << "Configuring for generic Allied Vision 1800 U-series" << std::endl;
        config.preferredPixelFormat = VmbPixelFormatBayerGR12;
        config.usbBandwidthLimit = 28000000;
        config.packetSize = 8192;
        config.streamBufferCount = 15;
        config.acquisitionTimeout = 45000;
    }
    else {
        std::cout << "Using default configuration for unknown camera model: " << cameraModel << std::endl;
    }

    return config;
}

void VimbaController::applyCameraConfig(VmbCPP::CameraPtr cam, const CameraConfig& config) {
    VmbErrorType err;

    // Apply USB bandwidth limit
    FeaturePtr pDeviceLinkThroughputLimit;
    err = cam->GetFeatureByName("DeviceLinkThroughputLimit", pDeviceLinkThroughputLimit);
    if (err == VmbErrorSuccess) {
        err = pDeviceLinkThroughputLimit->SetValue(config.usbBandwidthLimit);
        if (err == VmbErrorSuccess) {
            std::cout << "  - USB bandwidth limit: " << config.usbBandwidthLimit << " bytes/sec" << std::endl;
        }
    }

    // Apply packet size
    FeaturePtr pPacketSize;
    err = cam->GetFeatureByName("GVSPPacketSize", pPacketSize);
    if (err != VmbErrorSuccess) {
        err = cam->GetFeatureByName("PacketSize", pPacketSize);
    }
    if (err == VmbErrorSuccess) {
        VmbInt64_t minPacket, maxPacket;
        if (pPacketSize->GetRange(minPacket, maxPacket) == VmbErrorSuccess) {
            VmbInt64_t actualPacketSize = std::min(config.packetSize, maxPacket);
            actualPacketSize = std::max(actualPacketSize, minPacket);
            pPacketSize->SetValue(actualPacketSize);
            std::cout << "  - Packet size: " << actualPacketSize << " bytes" << std::endl;
        }
    }

    // Apply stream buffer count
    FeaturePtr pStreamBufferCount;
    err = cam->GetFeatureByName("StreamBufferCount", pStreamBufferCount);
    if (err == VmbErrorSuccess) {
        pStreamBufferCount->SetValue(config.streamBufferCount);
        std::cout << "  - Stream buffer count: " << config.streamBufferCount << std::endl;
    }

    // Apply pixel format
    FeaturePtr pFormatFeature;
    err = cam->GetFeatureByName("PixelFormat", pFormatFeature);
    if (err == VmbErrorSuccess) {
        err = pFormatFeature->SetValue(config.preferredPixelFormat);
        if (err != VmbErrorSuccess) {
            // Fallback to other 12-bit formats if preferred isn't available
            std::cout << "  - Preferred pixel format not available, trying 12-bit alternatives..." << std::endl;

            // Try only 12-bit Bayer formats (no 16-bit or 8-bit fallback)
            VmbPixelFormatType formats[] = {
                VmbPixelFormatBayerGR12, VmbPixelFormatBayerRG12,
                VmbPixelFormatBayerGB12, VmbPixelFormatBayerBG12,
                VmbPixelFormatMono12
            };

            bool formatFound = false;
            for (auto format : formats) {
                err = pFormatFeature->SetValue(format);
                if (err == VmbErrorSuccess) {
                    std::cout << "  - Using pixel format: 0x" << std::hex << format << std::dec << std::endl;
                    formatFound = true;
                    break;
                }
            }

            if (!formatFound) {
                std::cerr << "  - ERROR: No supported 12-bit pixel format found!" << std::endl;
            }
        } else {
            std::cout << "  - Pixel format: 0x" << std::hex << config.preferredPixelFormat << std::dec << std::endl;
        }
    }
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

bool VimbaController::saveImageAsTIFF(u_char* buffer, u_int width, u_int height, int bitsPerPixel, VmbPixelFormatType pixelFormat, const std::string& filename) {
    try {
        cv::Mat image;

        std::cout << "saveImageAsTIFF called with width=" << width << ", height=" << height
                  << ", bitsPerPixel=" << bitsPerPixel << ", pixelFormat=0x" << std::hex << pixelFormat << std::dec << std::endl;

        if (pixelFormat == VmbPixelFormatRgb8) {
            // RGB8 format - convert to grayscale
            cv::Mat rgb_image(height, width, CV_8UC3, buffer);
            cv::cvtColor(rgb_image, image, cv::COLOR_RGB2GRAY);
        }
        else if (pixelFormat == VmbPixelFormatMono8) {
            // Mono8 format - direct grayscale
            image = cv::Mat(height, width, CV_8UC1, buffer).clone();
        }
        else if (pixelFormat == VmbPixelFormatBayerGR8 ||
                 pixelFormat == VmbPixelFormatBayerRG8 ||
                 pixelFormat == VmbPixelFormatBayerGB8 ||
                 pixelFormat == VmbPixelFormatBayerBG8) {
            // Bayer 8-bit formats - need debayering
            cv::Mat temp(height, width, CV_8UC1, buffer);
            cv::Mat tempCopy = temp.clone();

            // Check data
            uint8_t* data8 = reinterpret_cast<uint8_t*>(buffer);
            uint8_t maxVal = 0;
            uint8_t minVal = 255;
            size_t nonZeroCount = 0;
            for (size_t i = 0; i < (width * height); i++) {
                uint8_t val = data8[i];
                if (val > 0) nonZeroCount++;
                maxVal = std::max(maxVal, val);
                minVal = std::min(minVal, val);
            }

            std::cout << "Bayer 8-bit data analysis: min=" << (int)minVal << ", max=" << (int)maxVal
                      << ", nonZero=" << nonZeroCount << "/" << (width*height) << std::endl;

            // Debayer to RGB
            int debayerCode;
            if (pixelFormat == VmbPixelFormatBayerRG8) {
                debayerCode = cv::COLOR_BayerRG2BGR;
            } else if (pixelFormat == VmbPixelFormatBayerGR8) {
                debayerCode = cv::COLOR_BayerGR2BGR;
            } else if (pixelFormat == VmbPixelFormatBayerGB8) {
                debayerCode = cv::COLOR_BayerGB2BGR;
            } else {
                debayerCode = cv::COLOR_BayerBG2BGR;
            }

            cv::cvtColor(tempCopy, image, debayerCode);
        }
        else if (pixelFormat == VmbPixelFormatMono12) {
            // Mono 12-bit format
            cv::Mat temp(height, width, CV_16UC1, buffer);

            // Check if data looks like 12-bit in 16-bit containers
            uint16_t* data16 = reinterpret_cast<uint16_t*>(buffer);
            uint16_t maxVal = 0;
            for (size_t i = 0; i < std::min((size_t)1000, (size_t)(width * height)); i++) {
                maxVal = std::max(maxVal, data16[i]);
            }

            if (maxVal > 4095) {
                // Data appears to be left-aligned 12-bit (shifted left by 4)
                temp.convertTo(image, CV_8UC1, 1.0/256.0);
                std::cout << "Treating as left-aligned 12-bit mono data (max sample: " << maxVal << ")" << std::endl;
            } else {
                // Data appears to be right-aligned 12-bit (0-4095 range)
                temp.convertTo(image, CV_8UC1, 1.0/16.0);
                std::cout << "Treating as right-aligned 12-bit mono data (max sample: " << maxVal << ")" << std::endl;
            }
        }
        else if (pixelFormat == VmbPixelFormatBayerGR12 ||
                 pixelFormat == VmbPixelFormatBayerRG12 ||
                 pixelFormat == VmbPixelFormatBayerGB12 ||
                 pixelFormat == VmbPixelFormatBayerBG12) {
            // Bayer 12-bit formats - need debayering
            // Clone the data to ensure we have a proper copy
            cv::Mat temp(height, width, CV_16UC1, buffer);
            cv::Mat tempCopy = temp.clone();

            // Check alignment
            uint16_t* data16 = reinterpret_cast<uint16_t*>(buffer);
            uint16_t maxVal = 0;
            uint16_t minVal = 65535;
            size_t nonZeroCount = 0;
            for (size_t i = 0; i < (width * height); i++) {
                uint16_t val = data16[i];
                if (val > 0) nonZeroCount++;
                maxVal = std::max(maxVal, val);
                minVal = std::min(minVal, val);
            }

            std::cout << "Bayer data analysis: min=" << minVal << ", max=" << maxVal
                      << ", nonZero=" << nonZeroCount << "/" << (width*height) << std::endl;

            // Normalize to 16-bit range for debayering
            cv::Mat normalized;
            if (maxVal > 4095) {
                // Already in full 16-bit range
                normalized = tempCopy;
                std::cout << "Treating as left-aligned 12-bit Bayer data (max sample: " << maxVal << ")" << std::endl;
            } else {
                // Scale up 12-bit to 16-bit range
                tempCopy.convertTo(normalized, CV_16UC1, 16.0);
                std::cout << "Treating as right-aligned 12-bit Bayer data (max sample: " << maxVal << ")" << std::endl;
            }

            // Debayer to RGB
            cv::Mat rgb;
            int debayerCode;
            if (pixelFormat == VmbPixelFormatBayerRG12) {
                debayerCode = cv::COLOR_BayerRG2BGR;
            } else if (pixelFormat == VmbPixelFormatBayerGR12) {
                debayerCode = cv::COLOR_BayerGR2BGR;
            } else if (pixelFormat == VmbPixelFormatBayerGB12) {
                debayerCode = cv::COLOR_BayerGB2BGR;
            } else {
                debayerCode = cv::COLOR_BayerBG2BGR;
            }

            cv::cvtColor(normalized, rgb, debayerCode);

            // Convert to 8-bit for saving
            rgb.convertTo(image, CV_8UC3, 1.0/256.0);
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

    // Get camera model for configuration
    std::string cameraModel;
    cam->GetModel(cameraModel);

    // Get camera-specific configuration
    CameraConfig config = getCameraConfig(cameraModel);

    std::cout << "\n========== Camera Acquisition Configuration ==========" << std::endl;
    std::cout << "Camera Model: " << cameraModel << std::endl;

    // Apply camera-specific configuration
    applyCameraConfig(cam, config);

    // Set acquisition mode to single frame
    FeaturePtr pAcquisitionMode;
    VmbErrorType err = cam->GetFeatureByName("AcquisitionMode", pAcquisitionMode);
    if (err == VmbErrorSuccess) {
        err = pAcquisitionMode->SetValue("SingleFrame");
        if (err != VmbErrorSuccess) {
            std::cerr << "Warning: Could not set AcquisitionMode to SingleFrame, err=" << err << std::endl;
        }
    }

    // Configure resolution
    if (config.useFullResolution) {
        FeaturePtr pOffsetX, pOffsetY, pWidth, pHeight;

        // Reset ROI offsets to 0
        if (cam->GetFeatureByName("OffsetX", pOffsetX) == VmbErrorSuccess) {
            pOffsetX->SetValue(0);
        }
        if (cam->GetFeatureByName("OffsetY", pOffsetY) == VmbErrorSuccess) {
            pOffsetY->SetValue(0);
        }

        // Set to maximum sensor resolution
        VmbInt64_t minWidth, maxWidth, minHeight, maxHeight;
        if (cam->GetFeatureByName("Width", pWidth) == VmbErrorSuccess) {
            if (pWidth->GetRange(minWidth, maxWidth) == VmbErrorSuccess) {
                VmbInt64_t fullWidth = (maxWidth / 2) * 2;  // Ensure even width
                pWidth->SetValue(fullWidth);
                std::cout << "  - Resolution: " << fullWidth;
            }
        }
        if (cam->GetFeatureByName("Height", pHeight) == VmbErrorSuccess) {
            if (pHeight->GetRange(minHeight, maxHeight) == VmbErrorSuccess) {
                VmbInt64_t fullHeight = (maxHeight / 2) * 2;  // Ensure even height
                pHeight->SetValue(fullHeight);
                std::cout << " x " << fullHeight << std::endl;
            }
        }
    }
    // Set exposure and gain
    FeaturePtr pExposureFeature, pGainFeature;

    err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
    if ( VmbErrorSuccess != err ) {
        err = cam->GetFeatureByName("ExposureTime", pExposureFeature);
    }

    if (VmbErrorSuccess == err) {
        err = pExposureFeature->SetValue(exposure);
        std::cout << "  - Exposure: " << exposure << " µs" << std::endl;
    }

    err = cam->GetFeatureByName("Gain", pGainFeature);
    if (VmbErrorSuccess == err) {
        err = pGainFeature->SetValue(gain);
        std::cout << "  - Gain: " << gain << std::endl;
    }

    std::cout << "======================================================" << std::endl;

    // Acquire frame with camera-specific timeout
    FramePtr frame;
    err = cam->AcquireSingleImage(frame, config.acquisitionTimeout);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    // Check frame status for incomplete transfers
    VmbFrameStatusType frameStatus;
    if (frame->GetReceiveStatus(frameStatus) == VmbErrorSuccess) {
        std::cout << "Frame receive status: " << frameStatus << std::endl;
        if (frameStatus != VmbFrameStatusComplete) {
            std::cerr << "WARNING: Frame is incomplete! Status=" << frameStatus << std::endl;
        }
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
        // Find the requested camera without printing
        for(size_t i = 0; i < cameras.size(); i++){
            std::string camName;
            cameras.at(i)->GetModel(camName);

            if(camName == capture_instructions.CameraId){
                cam = cameras.at(i);
                break;  // Found it, stop searching
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

            // Save image as TIFF
            std::string tiffFilename = "captured_image.tiff";
            saveImageAsTIFF(buffer, width, height, bitsPerPixel, pixelFormat, tiffFilename);

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
