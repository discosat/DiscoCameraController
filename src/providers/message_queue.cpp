#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <chrono>
#include "message_queue.hpp"
#include "types.hpp"
#include <string.h>
#include <iostream>

MessageQueue::MessageQueue(){}

MessageQueue::~MessageQueue(){}

int MessageQueue::createMemorySpace(size_t size){
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

void* MessageQueue::insertMemory(unsigned char *data, size_t size, int shm_id){
    void* shmaddr = shmat(shm_id, NULL, 0);
    memcpy(shmaddr, data, size); // Copy image batch data to shared memory

    return shmaddr;
}

bool MessageQueue::sendMessage(ImageBatchMessage batch){
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

    std::cout << "msgsnd: " << msgQueueId << " " << sizeof(batch)  - sizeof(long) << std::endl;

    if (msgsnd(msgQueueId, &batch, sizeof(batch) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        return false;
    }

    return true;
}

bool MessageQueue::SendImage(ImageBatch batch){
    int memspace;
    void* addr;

    std::cout << "1" << std::endl;
    if(memspace = createMemorySpace(batch.data_size) < 0){
        std::cout << "Failed to create memspace" << std::endl;
        return false;
    }

    std::cout << "2" << std::endl;
    addr = insertMemory(batch.data, batch.data_size, memspace);
    std::cout << "Addr: " << addr << std::endl;

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