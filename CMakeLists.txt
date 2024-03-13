cmake_minimum_required(VERSION 3.0)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-1/api/lib/cmake/vmb)

find_package(Vmb REQUIRED COMPONENTS CPP)
find_package(OpenCV REQUIRED)
find_library(csp PATHS /usr/local/include/csp /usr/local/lib/)

add_subdirectory(src/camera_control)
add_subdirectory(src/communication)
add_subdirectory(src/utils)

add_executable(Disco2CameraControl src/main.cpp)

target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP ${OpenCV_LIBS} libcamera_control libcommunication libutils csp)
target_include_directories(Disco2CameraControl PRIVATE ${CMAKE_SOURCE_DIR}/src/camera_control ${CMAKE_SOURCE_DIR}/src/communication ${CMAKE_SOURCE_DIR}/src/utils)
