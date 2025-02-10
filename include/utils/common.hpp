#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

typedef struct ImageBatch {
    long mtype;          /* message type to read from the message queue */
    int num_images;      /* amount of images */
    int batch_size;      /* size of the image batch */
    int shmid;         /* key to shared memory segment of image data */
    int pipeline_id;     /* id of pipeline to utilize for processing */
    u_char *data; /* address to image data (in shared memory) */
} ImageBatch;

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
    Unknown,
};

typedef struct CaptureMessage {
    std::string CameraId;       // camera id to use
    CameraType Type;            // camera type to use [IR, Vimba, Test, Unknown]
    uint32_t NumberOfImages;    // number of images to take
    uint32_t Exposure;          // exposure
    double ISO;                 // Gain/ISO
    uint32_t Interval;          // delay in microseconds
    uint32_t PipelineId;        // Pipeline to use
    uint32_t OBID;              // image batch unique identifier
} CaptureMessage;

// how many bytes in the beginning of the image buffer is allocated for metadata, which is just the size of the image
#define IMAGE_METADATA_SIZE 4

#define MSG_QUEUE_KEY 71

#endif