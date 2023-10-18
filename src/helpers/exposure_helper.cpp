#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "exposure_helper.hpp"

using namespace std;
using namespace cv;

double ExposureHelper::calculateEntropy(Mat image) {
    if (image.empty()) {
        cerr << "Error: Input image is empty." << endl;
        return 0.0;
    }

    if (image.channels() > 3) {
        cvtColor(image, image, COLOR_BGR2RGB);
    }

    int hist_r[256]={0}, hist_g[256]={0}, hist_b[256]={0};
    int totalPixels = image.rows * image.cols;

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i,j);
            int red = static_cast<int>(pixel[0]), green = static_cast<int>(pixel[1]), blue = static_cast<int>(pixel[2]);

            hist_r[red]++;
            hist_g[green]++;
            hist_b[blue]++;
        }
    }

    double total_r = 0.0, total_g = 0.0, total_b = 0.0;
    for (int i = 0; i < 256; i++) {
        if (hist_r[i] > 0) {
            double probability = static_cast<double>(hist_r[i]) / totalPixels;
            total_r += probability * log2(probability);
        }

        if (hist_g[i] > 0) {
            double probability = static_cast<double>(hist_g[i]) / totalPixels;
            total_g += probability * log2(probability);
        }

        if (hist_b[i] > 0) {
            double probability = static_cast<double>(hist_b[i]) / totalPixels;
            total_b += probability * log2(probability);
        }
    }

    return -total_r-total_g-total_b;
}