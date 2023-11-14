#!/bin/bash

base_exposure=5000

echo "starttime,endtime,cameraid,DeviceTemperature,AcquisitionFrameRate,ExposureTime,PixelFormat"

for i in {1..32}
do
    e=$((i*base_exposure))
    ./build/release/Disco2CameraControl -n 100 -e $e -f DeviceTemperature,AcquisitionFrameRate,ExposureTime,PixelFormat
    sleep 120
done