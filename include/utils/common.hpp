#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <string>

// typedef struct ImageBatch {
//     long mtype;          /* message type to read from the message queue */
//     int num_images;      /* amount of images */
//     int batch_size;      /* size of the image batch */
//     int shmid;         /* key to shared memory segment of image data */
//     int pipeline_id;     /* id of pipeline to utilize for processing */
//     u_char *data; /* address to image data (in shared memory) */
// } ImageBatch;

typedef enum
{
    STORAGE_MMAP,
    STORAGE_MEM,
    STORAGE_NOT_SET
} StorageMode;

typedef struct ImageBatch
{
    long mtype;               /* message type to read from the message queue */
    int num_images;           /* amount of images */
    int batch_size;           /* size of the image batch */
    int pipeline_id;          /* id of pipeline to utilize for processing */
    int priority;             /* priority of the image batch, e.g. max_latency from SLOs */
    unsigned char *data;      /* address to image data (in shared memory) */
    char filename[111];       /* filename of the image data */
    int shmid;                /* shared memory id for the image data */
    char uuid[37];            /* uuid of the image data */
    int progress;             /* index of the last processed module (-1 if not started) */
    StorageMode storage_mode; /* storage mode for the image data */
} ImageBatch;

typedef struct Image
{
    size_t height;
    size_t width;
    size_t size;
    size_t channels;
    size_t bpp;
    size_t timestamp;
    u_char *data;
} Image;

enum struct CameraType
{
    VMB,
    IR,
    TEST,
    Unknown,
};

typedef struct CaptureMessage
{
    std::string CameraId;          // camera id to use
    CameraType Type;               // camera type to use [IR, Vimba, Test, Unknown]
    uint32_t NumberOfImages;       // number of images to take
    uint32_t Exposure;             // exposure
    double ISO;                    // Gain/ISO
    uint32_t Interval;             // delay in microseconds
    uint32_t PipelineId;           // Pipeline to use
    uint32_t OBID;                 // image batch unique identifier
    uint32_t MaxProcessingLatency; // maximum processing latency in seconds
} CaptureMessage;

// how many bytes in the beginning of the image buffer is allocated for metadata, which is just the size of the image
#define IMAGE_METADATA_SIZE 4

#define MSG_QUEUE_KEY 71

#endif