#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdint.h>

typedef struct ImageBatch {
    long mtype;
    int height;
    int width;
    int channels;
    int num_images;
    size_t data_size;
    unsigned char *data;
} ImageBatch;

typedef struct ImageBatchMessage {
    long mtype;
    int height;
    int width;
    int channels;
    int num_images;
    int mem_key;
    size_t data_size;
} ImageBatchMessage;

#endif