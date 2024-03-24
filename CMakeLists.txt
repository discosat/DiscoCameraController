cmake_minimum_required(VERSION 3.0)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message("Target architecture is aarch64.")
    set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_HOST_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
    set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-4-ARM64/api/lib/cmake/vmb)
    find_library(csp)
else()
    set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-4-Linux/api/lib/cmake/vmb)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_THREAD_LIBS_INIT "-lpthread -lbsd")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(DIR_TO_BSD_LIB_FILE /usr/lib/x86_64-linux-gnu/libbsd.so)
set(DIR_TO_CSP_LIB_FILE /usr/local/lib/libcsp.so)

find_package(Vmb REQUIRED COMPONENTS CPP)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/camera_control)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/communication)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/message_queue)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/utils)

add_executable(Disco2CameraControl src/main.cpp)

target_include_directories(Disco2CameraControl PUBLIC 
    ${CMAKE_SOURCE_DIR}/src/camera_control 
    ${CMAKE_SOURCE_DIR}/src/communication
    ${CMAKE_SOURCE_DIR}/src/message_queue 
    ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(Disco2CameraControl PUBLIC 
    Vmb::CPP 
    ${CMAKE_THREAD_LIBS_INIT}
    libcamera_control  
    libutils libmq
    csp param libcommunication
)