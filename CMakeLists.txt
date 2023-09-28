cmake_minimum_required(VERSION 3.0)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

find_package(Vmb REQUIRED COMPONENTS CPP)

add_executable(Disco2CameraControl
    src/main.cpp
    src/VimbaProvider.cpp
    src/include/VimbaProvider.hpp
)

target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP )