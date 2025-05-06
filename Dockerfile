FROM ubuntu:22.04

# Set environment variable to suppress interactive prompts (like tzdata)
ENV DEBIAN_FRONTEND=noninteractive

# Install base build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    meson \
    ninja-build \
    libzmq3-dev \
    libbsd-dev \
    pkg-config \
    wget \
    curl \
    unzip \
    autoconf \
    libtool \
    software-properties-common \
    python3 \
    python3-pip \
    libopencv-dev && \
    ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata

# Install Protobuf 3.19.x from source
WORKDIR /opt
RUN git clone -b 3.19.x https://github.com/protocolbuffers/protobuf.git && \
    cd protobuf && \
    git submodule update --init --recursive && \
    ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Copy local source code (this assumes you're building from inside DiscoCameraController/)
WORKDIR /workspace
COPY . /workspace/DiscoCameraController
WORKDIR /workspace/DiscoCameraController

# Optional: Confirm we’re seeing the right file (debug only)
RUN grep iface src/communication/csp_server.c || echo "No iface found"

# Build the project
RUN chmod +x build.sh && ./build.sh

CMD ["bash"]
