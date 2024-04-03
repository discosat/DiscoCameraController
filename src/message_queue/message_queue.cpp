#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <chrono>
#include "message_queue.hpp"
#include "common.hpp"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <errors.hpp>

MessageQueue::MessageQueue(){}

MessageQueue::~MessageQueue(){}

int MessageQueue::createMemorySpace(size_t size){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    key_t key = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    int shm_id = 0;

    if ((shm_id = shmget(key, size, IPC_CREAT|0600 )) < 0) {
        return -1;
    } else {
        return shm_id;
    }
}

void* MessageQueue::insertMemory(unsigned char *data, size_t size, int shm_id){
    void* shmaddr = shmat(shm_id, NULL, 0);

    if (shmaddr == (void*)-1) {
        return NULL;
    }

    memcpy(shmaddr, data, size * sizeof(unsigned char)); // Copy image batch data to shared memory

    return shmaddr;
}

bool MessageQueue::sendMessage(ImageBatch batch){
    int msgQueueId;

    if ((msgQueueId = msgget(MSG_QUEUE_KEY, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        return false;
    }

    if (msgsnd(msgQueueId, &batch, sizeof(batch), 0) == -1) {
        perror("msgsnd");
        return false;
    }

    return true;
}

bool MessageQueue::SendImage(ImageBatch batch, u_int16_t* error){
    int memspace;
    void* addr;

    if((memspace = createMemorySpace(batch.batch_size)) < 0){
        *error = ERROR_CODE::MESSAGE_QUEUE_ERROR_MEMORY_SPACE_FAILURE;
        return false;
    }

    addr = insertMemory(batch.data, batch.batch_size, memspace);

    batch.mtype = 1;
    batch.shm_key  = memspace;

    std::cout << "shm_key: " << (void*)memspace << std::endl;

    if(addr != NULL && sendMessage(batch) && (shmdt(addr)) != -1){
        return true;
    } else {
        // cleanup
        shmdt(addr);
        shmctl(memspace, IPC_RMID, nullptr);
        *error = ERROR_CODE::MESSAGE_QUEUE_ERROR_INSERT_DATA_FAILURE;
        return false;
    }
}