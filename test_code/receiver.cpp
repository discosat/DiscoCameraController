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
#include "metadata.pb.hpp"

namespace fs = std::filesystem;

using namespace std;

typedef struct ImageBatch {
    long mtype;          /* message type to read from the message queue */
    int num_images;      /* amount of images */
    int batch_size;      /* size of the image batch */
    int shm_key;         /* key to shared memory segment of image data */
    int pipeline_id;     /* id of pipeline to utilize for processing */
    u_char *data; /* address to image data (in shared memory) */
} ImageBatch;

void readMem(ImageBatch msg){
    // Attach to the shared memory segment
    std::cout << msg.shm_key << std::endl;
    std::cout << msg.batch_size << std::endl;
    int shmkey = shmget(msg.shm_key, 0, 0);
    void* shared_memory = shmat(shmkey, nullptr, 0);

    // unable to attach -> error
    if (shared_memory == (void*)-1) {
        perror("shmat");
    }

    // // create a local copy of the data
    // uint localDataSize = msg.batch_size/msg.num_images;
    // unsigned char* local_data = new unsigned char[msg.batch_size];

    // // copy the shared data to the local copy
    // memcpy(local_data, ((unsigned char*)shared_memory), msg.batch_size);

    // Detach from the shared memory segment !VERY IMPORTANT!
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
    }

    // // Detach from the shared memory segment !VERY IMPORTANT!
    // shmctl(shmkey, IPC_RMID, nullptr);
    // std::string time = std::to_string(std::time(0));

    // // Read from shared memory
    // uint offset  = 0;
    // for(int i = 0; i < msg.num_images; i++){
    //     // get the data for image i into image buffer
    //     unsigned int metadata_size = local_data[offset];
    //     uchar* metadata_buffer = new uchar[metadata_size];
    //     memcpy(metadata_buffer, &local_data[offset+sizeof(metadata_size)], metadata_size);
    //     Metadata metadata;

    //     if (!metadata.ParseFromArray(metadata_buffer, metadata_size)) {
    //         std::cerr << "Failed to parse uchar array into message." << std::endl;
    //         return;
    //     }

    //     std::cout << "Image recieved with header: " << metadata_size << std::endl;
    //     std::cout << "Image height: " << metadata.height() << std::endl;
    //     std::cout << "Image width: " << metadata.width() << std::endl;
    //     std::cout << "Image size: " << metadata.size() << std::endl;
    //     std::cout << "Metadata size: " << metadata_size << std::endl;

    //     uchar* image_buffer = new uchar[metadata.size()];
    //     memcpy(image_buffer, &local_data[offset+sizeof(metadata_size)+metadata_size], metadata.size());
    //     offset+=sizeof(metadata_size)+metadata_size+metadata.size();

    //     cv::Mat rawImage(metadata.height(), metadata.width(), CV_16UC1, image_buffer);
    //     cv::Mat demosaicedImage;
    //     cv::cvtColor(rawImage, demosaicedImage, cv::COLOR_BayerGR2RGB);
    //     demosaicedImage *= 16;

    //     // save to path
    //     fs::path dir ("./");
    //     fs::path file ("image_" + std::to_string(std::time(0)) + "_" + std::to_string(i) + ".png");
    //     std::string full_path = (dir / file).string();
    //     imwrite(full_path, demosaicedImage);

    //     delete[] metadata_buffer;
    //     delete[] image_buffer;
    // }

    //delete[] local_data;
}

int main() {
    std::cout << "Simulator pipeline started" << std::endl;
    int msgQueueId;
    ImageBatch message;

    while(true) {
        // connect to the message queue
        if((msgQueueId = msgget(71, 0644)) == -1 || msgrcv(msgQueueId, &message, sizeof(message), 1, 0) == -1){
            continue;
        }

        cout << "Receiver: Message received: " << message.batch_size << " " << message.shm_key << endl;
        readMem(message);
    }

    // Remove the message queue
    if (msgctl(msgQueueId, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}