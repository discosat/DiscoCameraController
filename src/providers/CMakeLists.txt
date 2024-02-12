
find_package(Vmb REQUIRED COMPONENTS CPP)

add_library(libproviders STATIC vimba_provider.cpp message_queue.cpp)
target_include_directories(libproviders PRIVATE ${CMAKE_SOURCE_DIR}/src/include/providers ${CMAKE_SOURCE_DIR}/src/include)

target_link_libraries(libproviders PRIVATE Vmb::CPP)