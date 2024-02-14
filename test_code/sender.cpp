#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string.h>

typedef struct ImageBatch {
    long mtype;
    int height;
    int width;
    int channels;
    int num_images;
    size_t data_size;
    char *data;
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

int createMemorySpace(size_t size){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    key_t key = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    int shm_id = 0;

    if ((shm_id = shmget(key, size, IPC_CREAT | 0666 )) < 0) {
        perror("shmget");
        return -1;
    } else {
        return shm_id;
    }
}

void* insertMemory(char *data, size_t size, int shm_id){
    void* shmaddr = shmat(shm_id, NULL, 0);
    memcpy(shmaddr, data, size); // Copy image batch data to shared memory

    std::cout << "Data size:" << size << std::endl;

    return shmaddr;
}

bool sendMessage(ImageBatchMessage batch){
    key_t key;
    int msgQueueId;

    if ((key = ftok("/tmp", 'B')) == -1) {
        perror("ftok");
        return false;
    }

    if ((msgQueueId = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        return false;
    }

    std::cout << "msgsnd: " << batch.data_size << std::endl;

    if (msgsnd(msgQueueId, &batch, sizeof(batch), 0) == -1) {
        perror("msgsnd");
        return false;
    }

    return true;
}

bool SendImage(ImageBatch batch){
    int memspace;
    void* addr;

    if(memspace = createMemorySpace(batch.data_size) < 0){
        std::cout << "Failed to create memspace" << std::endl;
        return false;
    }

    addr = insertMemory(batch.data, batch.data_size, memspace);

    std::cout << addr << std::endl;

    ImageBatchMessage msg;
    msg.mtype = 1;
    msg.height = batch.height;
    msg.width = batch.width;
    msg.channels = batch.channels;
    msg.num_images = batch.num_images;
    msg.mem_key = memspace;
    msg.data_size = batch.data_size;

    if(addr != NULL && sendMessage(msg)){
        return true;
    } else {
        return false;
    }
}

int main(){
    char* buffer = "Hello";

    ImageBatch batch;
    batch.mtype = 0;
    batch.height = 20;
    batch.width = 20;
    batch.channels = 3;
    batch.num_images = 1;
    batch.data_size = 5;
    batch.data = buffer;

    SendImage(batch);
}