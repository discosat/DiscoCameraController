#include <iostream>
#include <vector>
#include <VmbCPP/VmbCPP.h>
#include <opencv2/opencv.hpp>
#include "VimbaProvider.hpp"
#include "exposure_helper.hpp"

using namespace Disco2Camera;

const float MAX_EXPOSURE = 1000000.0;
const float MIN_EXPOSURE = 1250.0;

float setExposure(VmbCPP::CameraPtr cam, bool saveImage, VimbaProvider* p){
    float currentExposure = MIN_EXPOSURE;
    float maximumEntropy = 0;
    float chosenExposure = 0;

    while(currentExposure < MAX_EXPOSURE){
        VmbCPP::FramePtr frame = p->AqcuireFrame(cam, currentExposure, 0);            
        unsigned int size, width, height;
        frame->GetBufferSize(size);
        frame->GetHeight(height);
        frame->GetWidth(width);
        
        unsigned char* buffer;
        frame->GetImage(buffer);

        cv::Mat img(height, width, CV_8UC3, buffer);
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        float currentEntropy = ExposureHelper::calculateEntropy(img);
        
        if (saveImage){
            cv::imwrite("out/"+std::to_string(currentExposure)+"_"+std::to_string(currentEntropy)+".png",img);
            if(currentEntropy > maximumEntropy){
                maximumEntropy = currentEntropy;
                chosenExposure = currentExposure;
            }
        } else {
            if(currentEntropy > maximumEntropy){
                maximumEntropy = currentEntropy;
            } else {
                return currentExposure/2;
            }
        }

        currentExposure *= 2;
    }

    return chosenExposure;
}

int main(){
    VimbaProvider* p = new VimbaProvider();
    std::vector<VmbCPP::CameraPtr> cameras = p->GetCameras();

    std::cout << "Got " << cameras.size() << " cameras" << std::endl;

    if(cameras.size() > 0){
        cameras.at(0)->Open(VmbAccessModeExclusive);
        float exposure = setExposure(cameras.at(0), true, p);
        std::cout << "Exposure found " << exposure << std::endl;
        cameras.at(0)->Close();
    }

    delete p; 
    return 0;
}

// AcquisitionFrameCount
// AcquisitionFrameRate
// AcquisitionFrameRateEnable
// AcquisitionFrameRateMode Basic
// AcquisitionMode Continuous
// AcquisitionStart
// AcquisitionStatus
// AcquisitionStatusSelector AcquisitionActive
// AcquisitionStop
// AdaptiveNoiseSuppressionFactor
// AutoModeRegionHeight
// AutoModeRegionOffsetX
// AutoModeRegionOffsetY
// AutoModeRegionSelector AutoModeRegion1
// AutoModeRegionWidth
// BalanceRatio
// BalanceRatioSelector Red
// BalanceWhiteAuto Off
// BalanceWhiteAutoRate
// BalanceWhiteAutoTolerance
// BinningHorizontal
// BinningHorizontalMode Sum
// BinningSelector Digital
// BinningVertical
// BinningVerticalMode Sum
// BlackLevel
// BlackLevelSelector All
// ChunkImage
// ColorInterpolation HighQualityLinear5X5
// ColorTransformationEnable
// ColorTransformationValue
// ColorTransformationValueSelector Gain00
// ContrastBrightLimit
// ContrastDarkLimit
// ContrastEnable
// ContrastShape
// ConvolutionMode Off
// CorrectionDataSize
// CorrectionEntryType
// CorrectionMode On
// CorrectionSelector DefectPixelCorrection
// CorrectionSet Preset
// CorrectionSetDefault Preset
// CounterDuration
// CounterEventActivation RisingEdge
// CounterEventSource Off
// CounterReset
// CounterResetActivation RisingEdge
// CounterResetSource Off
// CounterSelector Counter0
// CounterStatus CounterIdle
// CounterTriggerActivation RisingEdge
// CounterTriggerSource Off
// CounterValue
// CounterValueAtReset
// CustomConvolutionValue
// CustomConvolutionValueSelector Coefficient00
// DeviceFamilyName ALVIUM
// DeviceFirmwareID 44-0010102C
// DeviceFirmwareIDSelector Current
// DeviceFirmwareUploadType
// DeviceFirmwareVersion 11.0.9CF0C21E
// DeviceFirmwareVersionSelector Current
// DeviceGenCPVersionMajor
// DeviceGenCPVersionMinor
// DeviceIndicatorLuminance
// DeviceIndicatorMode Active
// DeviceLinkCommandTimeout
// DeviceLinkSpeed
// DeviceLinkThroughputLimit
// DeviceLinkThroughputLimitMode On
// DeviceManufacturerInfo 0-D2-0-263209-0-0
// DeviceModelName 1800 U-500c
// DevicePowerSavingMode Disabled
// DeviceReset
// DeviceSFNCVersionMajor
// DeviceSFNCVersionMinor
// DeviceSFNCVersionSubMinor
// DeviceScanType Areascan
// DeviceSerialNumber 04L65
// DeviceTLVersionMajor
// DeviceTLVersionMinor
// DeviceTemperature
// DeviceTemperatureSelector Mainboard
// DeviceUserID
// DeviceVendorName Allied Vision
// DeviceVersion 13885
// ExposureActiveMode FlashWindow
// ExposureAuto Off
// ExposureAutoMax
// ExposureAutoMin
// ExposureMode Timed
// ExposureTime
// FileAccessBuffer
// FileAccessLength
// FileAccessOffset
// FileOpenMode Read
// FileOperationExecute
// FileOperationResult
// FileOperationSelector Open
// FileOperationStatus Success
// FileProcessStatus None
// FileSelector UserData
// FileSize
// FileStatus Closed
// Gain
// GainAuto Off
// GainAutoMax
// GainAutoMin
// GainSelector All
// Gamma
// Height
// HeightMax
// Hue
// IntensityAutoPrecedence Minimizenoise
// IntensityControllerAlgorithm Mean
// IntensityControllerOutliersBright
// IntensityControllerOutliersDark
// IntensityControllerRate
// IntensityControllerRegion FullImage
// IntensityControllerSelector IntensityController1
// IntensityControllerTarget
// IntensityControllerTolerance
// LUTEnable
// LUTIndex
// LUTSelector Luminance
// LUTValue
// LineDebounceDuration
// LineDebounceMode Off
// LineInverter
// LineMode Input
// LineSelector Line0
// LineSource
// LineStatus
// LineStatusAll
// OffsetX
// OffsetY
// PayloadSize
// PixelFormat RGB8
// PixelSize Bpp24
// ReverseX
// ReverseY
// Saturation
// SensorBitDepth Bpp10
// SensorHeight
// SensorShutterMode RollingShutter
// SensorWidth
// SequencerConfigurationMode Off
// SequencerFeatureEnable
// SequencerFeatureSelector Width
// SequencerMode Off
// SequencerPathSelector Path0
// SequencerSetActive
// SequencerSetLoad
// SequencerSetNext
// SequencerSetSave
// SequencerSetSelector Set0
// SequencerSetStart
// SequencerTriggerActivation RisingEdge
// SequencerTriggerSource Off
// SerialBaudRate
// SerialHubEnable
// SerialParityBit
// SerialRxData
// SerialRxSize
// SerialRxWaiting
// SerialStopBits
// SerialTxData
// SerialTxLock
// SerialTxRemaining
// SerialTxSize
// Sharpness
// SoftwareSignalPulse
// SoftwareSignalSelector SoftwareSignal0
// TLParamsLocked
// TestPendingAck
// TimerDelay
// TimerDuration
// TimerReset
// TimerSelector Timer0
// TimerStatus TimerTriggerWait
// TimerTriggerActivation RisingEdge
// TimerTriggerSource Off
// TimestampLatch
// TimestampLatchValue
// TimestampReset
// TransferControlMode Basic
// TransferQueueCurrentBlockCount
// TransferQueueMaxBlockCount
// TransferSelector Stream0
// TriggerActivation RisingEdge
// TriggerDelay
// TriggerMode Off
// TriggerSelector FrameStart
// TriggerSoftware
// TriggerSource Software
// UserSetDefault Default
// UserSetLoad
// UserSetSave
// UserSetSelector Default
// Width
// WidthMax