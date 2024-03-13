
find_package(Vmb REQUIRED COMPONENTS CPP)

set(SOURCES
    camera_controller.cpp
)

set(HEADERS
    camera_controller.hpp
)

add_library(libcamera_control ${SOURCES} ${HEADERS})

target_include_directories(libcamera_control PRIVATE ${CMAKE_SOURCE_DIR}/src/utils)

target_link_libraries(libcamera_control PRIVATE Vmb::CPP)