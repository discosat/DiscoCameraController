
find_package(Vmb REQUIRED COMPONENTS CPP)

set(SOURCES
    csp_server_posix.cpp
    csp_server.cpp
    message_queue.cpp
)

set(HEADERS
    csp_server_posix.hpp
    csp_server.hpp
    message_queue.hpp
)

add_library(libcommunication ${SOURCES} ${HEADERS})

target_include_directories(libcommunication PRIVATE ${CMAKE_SOURCE_DIR}/src/utils ${CMAKE_SOURCE_DIR}/src/camera_control)

target_link_libraries(libcommunication PRIVATE Vmb::CPP)