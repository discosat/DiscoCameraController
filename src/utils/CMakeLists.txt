#find_package(OpenCV REQUIRED)

set(SOURCES
#    exposure_helper.cpp
    csp_parser.cpp
)

set(HEADERS
#    exposure_helper.hpp
    csp_parser.hpp
    common.hpp
)

add_library(libutils ${SOURCES} ${HEADERS})

target_include_directories(libutils PRIVATE ${CMAKE_SOURCE_DIR}/src/camera_control)

#target_link_libraries(libutils PRIVATE ${OpenCV_LIBS} Vmb::CPP)
target_link_libraries(libutils PRIVATE Vmb::CPP)