cmake_minimum_required(VERSION 3.0)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

find_package(Vmb REQUIRED COMPONENTS CPP)
find_package(OpenCV REQUIRED)

add_subdirectory(src/helpers)
add_subdirectory(src/providers)

add_executable(Disco2CameraControl src/main.cpp)

target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP ${OpenCV_LIBS} libproviders libhelpers)
target_include_directories(Disco2CameraControl PRIVATE ${CMAKE_SOURCE_DIR}/src/include/helpers ${CMAKE_SOURCE_DIR}/src/include/providers)