cmake_minimum_required(VERSION 3.0)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message("Target architecture is aarch64.")
    set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    #set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
    #set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)
    # set(CMAKE_FIND_ROOT_PATH ${STAGING_DIR_HOST} ${STAGING_DIR_NATIVE} ${CROSS_DIR})
    # set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    # SET(CMAKE_SKIP_BUILD_RPATH  TRUE)
    # SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    # SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

    # # for libraries and headers in the target directories
    # set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    # set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    # set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
    set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-4-ARM64/api/lib/cmake/vmb)
    find_library(csp)
else()
    set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-4-Linux/api/lib/cmake/vmb)
    find_library(csp PATHS /usr/local/include/csp /usr/local/lib/)
endif()

set(CMAKE_CXX_STANDARD 20)

#find_package(Threads REQUIRED)
find_package(Vmb REQUIRED COMPONENTS CPP)
#find_package(OpenCV REQUIRED)
#add_subdirectory(${CMAKE_SOURCE_DIR}/lib/libcsp)

add_subdirectory(src/camera_control)
add_subdirectory(src/communication)
add_subdirectory(src/utils)

add_executable(Disco2CameraControl src/main.cpp)

target_compile_options(Disco2CameraControl PUBLIC "-pthread")

#target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP ${OpenCV_LIBS} libcamera_control libcommunication libutils csp)
target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP libcamera_control libcommunication libutils csp ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(Disco2CameraControl PRIVATE ${CMAKE_SOURCE_DIR}/src/camera_control ${CMAKE_SOURCE_DIR}/src/communication ${CMAKE_SOURCE_DIR}/src/utils)
