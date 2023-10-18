find_package(OpenCV REQUIRED)

add_library(libhelpers STATIC exposure_helper.cpp)
target_include_directories(libhelpers PRIVATE ${CMAKE_SOURCE_DIR}/src/include/helpers)

target_link_libraries(libhelpers PRIVATE ${OpenCV_LIBS})