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

            // Validate frame completeness before processing
            VmbFrameStatusType frameStatus;
            if (frame->GetReceiveStatus(frameStatus) == VmbErrorSuccess) {
                if (frameStatus != VmbFrameStatusComplete) {
                    std::cerr << "Warning: Frame not complete! Status: " << frameStatus << std::endl;
                    if (frameStatus == VmbFrameStatusIncomplete) {
                        throw std::runtime_error("Frame incomplete - possible USB transfer issue");
                    }
                }
            } else {
                std::cerr << "Warning: Could not get frame status" << std::endl;
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
            
            // Calculate expected buffer size for validation
            size_t expectedSize;
            if (bitsPerPixel == 12) {
                // For 12-bit packed: each 2 pixels use 3 bytes
                expectedSize = ((width * height + 1) / 2) * 3;
            } else {
                expectedSize = width * height * channels * ((bitsPerPixel + 7) / 8);
            }
            
            // Validate buffer size matches expected size
            if (bufferSize != expectedSize) {
                std::cout << "Buffer size validation: Expected " << expectedSize 
                          << ", Got " << bufferSize 
                          << " (difference: " << (int64_t)bufferSize - (int64_t)expectedSize << ")" << std::endl;
            }

            Image img;
            img.size = bufferSize;  // Use actual buffer size from VimbaX
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
                      << ", buffer size: " << bufferSize << " bytes" 
                      << ", expected: " << expectedSize << " bytes" << std::endl;
            
            // Copy the actual image data from buffer to img.data
            std::memcpy(img.data, buffer, bufferSize);
            
            // Validate that the buffer contains reasonable data
            size_t nonZeroBytes = 0;
            for (size_t i = 0; i < std::min(bufferSize, (u_int)1000); i++) {
                if (buffer[i] != 0) nonZeroBytes++;
            }
            std::cout << "Buffer validation: " << nonZeroBytes << "/1000 bytes non-zero in first 1KB" << std::endl;

      	    std::ofstream outfile("buffer_dump.bin", std::ios::binary);
	    if (outfile.is_open()) {
	        outfile.write(reinterpret_cast<const char*>(img.data), bufferSize);
	        outfile.close();
	        std::cout << "Buffer dumped to buffer_dump.bin (size: " << bufferSize << " bytes)" << std::endl;
      	    }
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
