#!/bin/bash

num_images=(1 10 25 50 100 150 200 250 500 750 1000)
exposures=(10000 50000 100000 200000)

for num in "${num_images[@]}"; do
    for exp in "${exposures[@]}"; do
        ./build/Disco2CameraControl -D -M "CAMERA_TYPE=TEST;CAMERA_ID=TEST;NUM_IMAGES=$num;EXPOSURE=$exp;" > /dev/null 2>&1 &
        pid=$!

        while true; do
            if ps -p $pid > /dev/null; then
                time=$(date +%s%N | cut -b1-13)
                memory_bytes=$(ps -p $pid -o rss=)
                cpu_percent=$(ps -p $pid -o %cpu=)
                echo "${memory_bytes},${cpu_percent},${time}"
            else
                break
            fi
        done
    done
done