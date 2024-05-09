#!/bin/bash

num_images=(75) # (75 100 125 150 175 180 190 200)
exposures=(10000 50000)

for num in "${num_images[@]}"; do
    for exp in "${exposures[@]}"; do
        ./build/Disco2CameraControl -D -M "CAMERA_TYPE=VMB;CAMERA_ID=;NUM_IMAGES=$num;EXPOSURE=$exp;" > /dev/null 2>> "errors_${exp}_${num}.csv" &
        pid=$!
        echo "memory_bytes,shmem,cpu_percent,swap_used,swap_total,mem_used_percent,time" >> "experiment_${exp}_${num}.csv"

        while true; do
            if ps -p $pid > /dev/null; then
                time=$(date +%s%N | cut -b1-13)
                memory_bytes=$(ps -p $pid -o rss=)
                shmem=$(grep Shmem: /proc/meminfo | awk '{print $2}')
                cpu_percent=$(ps -p $pid -o %cpu=)
                swap_total=$(grep SwapTotal /proc/meminfo | awk '{print $2}')
                swap_used=$(grep SwapFree /proc/meminfo | awk '{print $2}')
                mem_available=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
                mem_total=$(grep MemTotal /proc/meminfo | awk '{print $2}')
                mem_used_percent=$(awk "BEGIN {print (($mem_total - $mem_available) / $mem_total) * 100}")
                vmem_used_percent=$(vmstat | tail -1 | awk '{print $7}')
                
                echo "${memory_bytes},${shmem},${cpu_percent},${swap_used},${swap_total},${mem_used_percent},${time}" >> "experiment_${exp}_${num}.csv"
            else
                wait $pid
                exit_status=$?
                if [ $exit_status -ne 0 ]; then
                    echo "Process was killed." >> "experiment_${exp}_${num}.csv"
                fi
                break
            fi
        done
    done
done