#!/bin/bash

num_images=(1 5 10 15 20 30 40 50 75 100 125 150 175 180 190 200)
exposures=(10000 50000 100000 150000)

for num in "${num_images[@]}"; do
    for exp in "${exposures[@]}"; do
        ./build/Disco2CameraControl -D -M "CAMERA_TYPE=TEST;CAMERA_ID=;NUM_IMAGES=$num;EXPOSURE=$exp;" > /dev/null 2>> "errors_${exp}_${num}.csv" &
        pid=$!
        echo "memory_bytes,cpu_percent,swap_used,swap_total,mem_used_percent,time" >> "experiment_${exp}_${num}.csv"

        while true; do
            if ps -p $pid > /dev/null; then
                time=$(date +%s%N | cut -b1-13)
                memory_bytes=$(ps -p $pid -o rss=)
                cpu_percent=$(ps -p $pid -o %cpu=)
                swap_total=$(grep SwapTotal /proc/meminfo | awk '{print $2}')
                swap_used=$(grep SwapFree /proc/meminfo | awk '{print $2}')
                mem_available=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
                mem_total=$(grep MemTotal /proc/meminfo | awk '{print $2}')
                mem_used_percent=$(awk "BEGIN {print (($mem_total - $mem_available) / $mem_total) * 100}")
                echo "${memory_bytes},${cpu_percent},${swap_used},${swap_total},${mem_used_percent},${time}" >> "experiment_${exp}_${num}.csv"
            else
                break
            fi
        done
    done
done