.PHONY: all clean build setup

all: build

current_dir := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

clean:
	rm -rf build

setup:
	$(current_dir)lib/VimbaX_2023-1/VimbaUSBTL_Install.sh

build:
	mkdir -p ./build/release && \
	cmake -S ./ -B ./build/release -DCMAKE_BUILD_TYPE=Release && \
	cd ./build/release && \
	make
