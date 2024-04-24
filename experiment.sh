#!/bin/bash

num_images=(1 10 25 50 100 150 200 250 300 350 400)
exposures=(10000 50000 100000 200000)

for num in "${num_images[@]}"; do
    for exp in "${exposures[@]}"; do
        ./build/Disco2CameraControl -D -M "CAMERA_TYPE=VMB;CAMERA_ID=1800 U-500c;NUM_IMAGES=$num;EXPOSURE=$exp;" > /dev/null 2>&1 &
        pid=$!
        echo "memory_bytes,cpu_percent,time" >> "experiment_${exp}_${num}.csv"

        while true; do
            if ps -p $pid > /dev/null; then
                time=$(date +%s%N | cut -b1-13)
                memory_bytes=$(ps -p $pid -o rss=)
                cpu_percent=$(ps -p $pid -o %cpu=)
                echo "${memory_bytes},${cpu_percent},${time}" >> "experiment_${exp}_${num}.csv"
            else
                break
            fi
        done
    done
done