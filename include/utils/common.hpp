#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

typedef struct ImageBatch {
    long mtype;          /* message type to read from the message queue */
    int height;          /* height of images */
    int width;           /* width of images */
    int channels;        /* channels of images */
    int num_images;      /* amount of images */
    int batch_size;      /* size of the image batch */
    int shm_key;         /* key to shared memory segment of image data */
    int pipeline_id;     /* id of pipeline to utilize for processing */
    unsigned char *data; /* address to image data (in shared memory) */
} ImageBatch;

typedef struct ImageBatchMessage {
    long mtype;
    unsigned int height;
    unsigned int width;
    unsigned int channels;
    unsigned int num_images;
    unsigned int batch_size;
    int shm_key;
} ImageBatchMessage;

typedef struct CaptureMessage {
    size_t NumberOfImages;
    int Exposure;
    int Interval;
    float ISO;
    std::string Camera;
} CaptureMessage;

// how many bytes in the beginning of the image buffer is allocated for metadata, which is just the size of the image
#define IMAGE_METADATA_SIZE 4

#endif