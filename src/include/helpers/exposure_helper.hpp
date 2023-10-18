#include <opencv2/opencv.hpp>
#include <cmath>

#ifndef EXPOSUREHELPER_H
#define EXPOSUREHELPER_H

class ExposureHelper{
    public:
        static double calculateEntropy(cv::Mat image);
};

#endif