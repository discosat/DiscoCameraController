#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <ctime>
#include <bits/stdc++.h>
#include <chrono>

namespace fs = std::filesystem;

using namespace std;

typedef struct ImageBatch {
    unsigned int height;
    unsigned int width;
    unsigned int channels;
    unsigned int num_images;
    unsigned int data_size;
    int shm_key;
    unsigned char *data;
} ImageBatch;

typedef struct ImageBatchMessage {
    long mtype;
    unsigned int height;
    unsigned int width;
    unsigned int channels;
    unsigned int num_images;
    unsigned int data_size;
    int shm_key;
} ImageBatchMessage;

void readMem(ImageBatchMessage msg){
    int shmid = msg.shm_key;

    // Attach to the shared memory segment
    void* shared_memory = shmat(msg.shm_key, nullptr, 0);

    // unable to attach -> error
    if (shared_memory == (void*)-1) {
        perror("shmat");
    }

    // create a local copy of the data
    uint localDataSize = msg.data_size/msg.num_images;
    unsigned char* local_data = new unsigned char[msg.data_size];

    // copy the shared data to the local copy
    memcpy(local_data, ((unsigned char*)shared_memory), msg.data_size);

    // Detach from the shared memory segment !VERY IMPORTANT!
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
    }

    // Detach from the shared memory segment !VERY IMPORTANT!
    shmctl(shmid, IPC_RMID, nullptr);

    // Read from shared memory
    for(int i = 0; i < msg.num_images; i++){
        // get the data for image i into image buffer
        unsigned char* image_data = new unsigned char[localDataSize];
        memcpy(image_data, (void*)(&local_data[i * localDataSize]), localDataSize);

        cv::Mat img(msg.height, msg.width, CV_8UC3, image_data);
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

        // save to path
        fs::path dir ("./");
        fs::path file ("image_" + std::to_string(std::time(0)) + "_" + std::to_string(i) + ".png");
        std::string full_path = (dir / file).string();
        imwrite(full_path, img);
        delete[] image_data;
    }
}

int main() {
    key_t key;
    int msgQueueId;
    ImageBatchMessage message;

    if ((key = ftok("/tmp", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    while(true) {
        // connect to the message queue
        if((msgQueueId = msgget(key, 0644)) == -1 || msgrcv(msgQueueId, &message, sizeof(message), 1, 0) == -1){
            continue;
        }

        cout << "Receiver: Message received: " << message.data_size << " " << message.shm_key << endl;
        readMem(message);
    }

    // Remove the message queue
    if (msgctl(msgQueueId, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}
