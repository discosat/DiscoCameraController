#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

typedef struct ImageBatch {
    long mtype;          /* message type to read from the message queue */
    int num_images;      /* amount of images */
    int batch_size;      /* size of the image batch */
    int shm_key;         /* key to shared memory segment of image data */
    int pipeline_id;     /* id of pipeline to utilize for processing */
    u_char *data; /* address to image data (in shared memory) */
} ImageBatch;

typedef struct ImageBatchMessage {
    long mtype;
    u_int height;
    u_int width;
    u_int channels;
    u_int num_images;
    u_int batch_size;
    int shm_key;
} ImageBatchMessage;

typedef struct Image {
    size_t height;
    size_t width;
    size_t size;
    size_t channels;
    size_t bpp;
    size_t timestamp;
    u_char* data;
} Image;

enum struct CameraType {
    VMB,
    IR,
    TEST,
    Unkown,
};

typedef struct CaptureMessage {
    std::string CameraId;   // camera id to use
    CameraType Type;        // camera type to use [IR, Vimba, test, etc]
    size_t NumberOfImages;  // number of images to take
    u_int Exposure;         // exposure
    float ISO;              // Gain/ISO
    u_int Interval;         // delay in microseconds
    u_int PipelineId;       // Pipeline to use
} CaptureMessage;

// how many bytes in the beginning of the image buffer is allocated for metadata, which is just the size of the image
#define IMAGE_METADATA_SIZE 4

#define MSG_QUEUE_KEY 71

#endif