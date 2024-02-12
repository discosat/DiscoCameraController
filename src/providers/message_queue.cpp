#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <chrono>
#include "message_queue.hpp"
#include "types.hpp"
#include <string.h>

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

bool MessageQueue::sendMessage(ImageBatch batch){
    key_t key;
    int msgQueueId;

    if ((key = ftok("DISCO2IMAGE", 'B')) == -1) {
        perror("ftok");
        return -1;
    }

    if ((msgQueueId = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        return -1;
    }

    if (msgsnd(msgQueueId, &batch, sizeof(batch), 0) == -1) {
        perror("msgsnd");
        return -1;
    }
}

bool MessageQueue::SendImage(ImageBatch batch){
    int memspace;
    void* addr;

    if(memspace = createMemorySpace(batch.data_size) < 0){
        return false;
    }

    addr = insertMemory(batch.data, batch.data_size, memspace);

    if(addr != NULL){// && sendMessage(batch)){
        return shmdt(addr) == -1;
    } else {
        return false;
    }
}