#include "capture_controller.hpp"
#include <iostream>
#include <memory>
#include "errors.hpp"
#include <cmath>

CaptureController::CaptureController(){
    this->mq = new MessageQueue();
}

CaptureController::~CaptureController(){
    delete this->mq;
}

void CaptureController::Capture(CaptureMessage capture_instructions, u_int16_t* error){
    if(capture_instructions.NumberOfImages == 0){
        *error = ERROR_CODE::PARSING_ERROR_NUM_IMAGES_INVALID;
        return;
    }
    
    std::cout << "New capture instructions:" << std::endl;
    std::cout << "\tCamera: \"" << capture_instructions.CameraId << "\"" << std::endl;
    std::cout << "\tExposure: " << capture_instructions.Exposure << std::endl;
    std::cout << "\tISO: " << capture_instructions.ISO << std::endl;
    std::cout  << "\tNumber of images: " << capture_instructions.NumberOfImages << std::endl;
    
    std::unique_ptr<CameraController> controller = CaptureController::CreateControllerInstance(capture_instructions.Type);

    if(capture_instructions.Exposure == 0){
        capture_instructions.Exposure = set_exposure(std::move(controller), capture_instructions);
    }

    if(controller == nullptr){
        *error = ERROR_CODE::PARSING_ERROR_CAMERA_TYPE_INVALID;
        return;
    }

    auto images = controller->Capture(capture_instructions, error);

    if(*error != ERROR_CODE::SUCCESS){
        return;
    }

    u_int image_size = images.at(0).size, width = images.at(0).width, height = images.at(0).height;
    u_int bufferSize = image_size + IMAGE_METADATA_SIZE;
    unsigned char* total_buffer = new unsigned char[bufferSize*capture_instructions.NumberOfImages];

    for(size_t i = 0; i < images.size(); i++){
        unsigned int offset = i*bufferSize;

        std::memcpy((void*)(&total_buffer[offset]), &image_size, sizeof(unsigned int));
        std::memcpy((void*)(&total_buffer[offset+IMAGE_METADATA_SIZE]), images.at(i).data, image_size * sizeof(unsigned char));
    }

    ImageBatch batch;
    batch.height = height;
    batch.width = width;
    batch.channels = 1;
    batch.num_images = capture_instructions.NumberOfImages;
    batch.batch_size = bufferSize*capture_instructions.NumberOfImages;
    batch.data = total_buffer;

    if(mq->SendImage(batch, error)){
        std::cout << "Sending image was successful" << std::endl;
    } else {
        std::cout << "Sending image was unsuccessful" << std::endl;
    }
}

double CaptureController::calculateEntropy(Image image) {
    if (image.size == 0) {
        return 0.0;
    }

    int hist[256]={0};

    // needed number of bytes to store BPP
    size_t bytes = image.bpp/8;
    if(image.bpp%8)bytes++;
    using UIntType = uint8_t;
    
    // if image data uses 2 bytes
    if(bytes == 2){
        using UIntType = uint16_t;
    }

    // find max of BPP
    const size_t bpp_max = (1 << image.bpp) - 1;
    const size_t byte_max =  255;
    const double scale_factor = byte_max/byte_max;
    const size_t total_pixels = image.size/bytes;

    UIntType* data = reinterpret_cast<UIntType*>(image.data);

    for(size_t i = 0; i < total_pixels; i++){
        UIntType pixel = static_cast<UIntType>(data[i]);
        uint8_t value = static_cast<uint8_t>(pixel * scale_factor);
        hist[value]++;
    }

    double total = 0.0;
    for(uint16_t i = 0; i < 256; i++){
        if(hist[i] > 0){
            double probability = static_cast<double>(hist[i]) / total_pixels;
            total += probability * log2(probability);
        }
    }
    return -total;
}

size_t CaptureController::set_exposure(std::unique_ptr<CameraController> controller, CaptureMessage cap_msg){
    float currentExposure = EXPOSURE_START, lastEntropy = -1, lastExposure = -1, slope = 1;
    cap_msg.NumberOfImages = 1;
    u_int16_t error = 0;
    size_t steps = 0;

    while(currentExposure < MAX_EXPOSURE && steps < STEPS){
        cap_msg.Exposure = currentExposure;
        Image img = controller->Capture(cap_msg, &error).at(0);
        float currentEntropy = calculateEntropy(img);

        if(lastEntropy == -1){
            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE;
        } else {
            float exposure_delta = currentExposure - lastExposure;
            float entropy_delta = currentEntropy - lastEntropy;
            slope = (entropy_delta/MAX_ENTROPY)/(exposure_delta/MAX_EXPOSURE); // normalized slope

            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE*slope;
        }

        steps++;
    }

    return std::round(currentExposure);
}