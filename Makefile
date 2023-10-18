.PHONY: all clean debug release setup

all: debug

current_dir := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

clean:
	rm -rf build

setup:
	$(current_dir)lib/VimbaX_2023-1/VimbaUSBTL_Install.sh

release:
	mkdir -p ./build/release && \
	export Vmb_DIR=$(current_dir)lib/VimbaX_2023-1/api/lib/cmake/vmb/ && \
	cmake -S ./ -B ./build/release -DCMAKE_BUILD_TYPE=Release && \
	cd ./build/release && \
	make

debug:
	mkdir -p ./build/release && \
	export Vmb_DIR=$(current_dir)lib/VimbaX_2023-1/api/lib/cmake/vmb/ && \
	cmake -S ./ -B ./build/release -DCMAKE_BUILD_TYPE=Debug && \
	cd ./build/release && \
	make
