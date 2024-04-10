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

void readMem(ImageBatch msg){
    // Attach to the shared memory segment
    std::cout << msg.height << std::endl;
    std::cout << msg.width << std::endl;
    std::cout << msg.shm_key << std::endl;
    std::cout << msg.batch_size << std::endl;
    int shmkey = shmget(msg.shm_key, 0, 0);
    void* shared_memory = shmat(shmkey, nullptr, 0);

    // unable to attach -> error
    if (shared_memory == (void*)-1) {
        perror("shmat");
    }

    // create a local copy of the data
    uint localDataSize = msg.batch_size/msg.num_images;
    unsigned char* local_data = new unsigned char[msg.batch_size];

    // copy the shared data to the local copy
    memcpy(local_data, ((unsigned char*)shared_memory), msg.batch_size);

    // Detach from the shared memory segment !VERY IMPORTANT!
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
    }

    // Detach from the shared memory segment !VERY IMPORTANT!
    shmctl(shmkey, IPC_RMID, nullptr);
    std::string time = std::to_string(std::time(0));

    // Read from shared memory
    for(int i = 0; i < msg.num_images; i++){
        // get the data for image i into image buffer
        size_t header_size = 4;
        size_t image_size = localDataSize-header_size;
        unsigned char* image_data = new unsigned char[image_size];
        unsigned int image_header = *((unsigned int*)&local_data[(i * localDataSize)]);
        memcpy(image_data, (void*)(&local_data[(i * localDataSize)+header_size]), image_size);

        std::cout << "Image recieved with header: " << image_header << std::endl;
        std::cout << "Image height: " << msg.height << std::endl;
        std::cout << "Image width: " << msg.width << std::endl;
        std::cout << "Image size: " << image_size << std::endl;

        // cv::Mat rawImage(msg.height, msg.width, CV_16UC1, image_data);
        // rawImage *= 16; // scale image to use 16 bits
        // cv::Mat demosaicedImage;
        // cv::cvtColor(rawImage, demosaicedImage, cv::COLOR_BayerGR2BGR);

        // // save to path
        // fs::path dir ("./");
        // fs::path file ("image_" + std::to_string(std::time(0)) + "_" + std::to_string(i) + ".png");
        // std::string full_path = (dir / file).string();
        // imwrite(full_path, demosaicedImage);
        // delete[] image_data;

        fs::path dir ("./");
        fs::path file ("image_" + time + "_" + std::to_string(i) + "_12bit.bayerRG");
        std::string full_path = (dir / file).string();
        std::cout << full_path << std::endl;

        // Open a file for binary writing
        std::ofstream outFile(full_path, std::ios::out | std::ios::binary);

        // Check if the file opened successfully
        if (!outFile) {
            std::cerr << "Error opening file for writing!" << std::endl;
            return;
        }

        // Write the raw data to the file
        outFile.write(reinterpret_cast<char*>(image_data), localDataSize);

        // Close the file
        outFile.close();

        std::cout << "Binary data saved to file successfully." << std::endl;
        delete[] image_data;
    }

    delete[] local_data;
}

int main() {
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