#include <opencv2/opencv.hpp>
#include <cmath>
#include "vimba_controller.hpp"

#ifndef EXPOSUREHELPER_H
#define EXPOSUREHELPER_H

double calculateEntropy(cv::Mat image);

float set_exposure(VmbCPP::CameraPtr cam, VimbaController* p);

#endif