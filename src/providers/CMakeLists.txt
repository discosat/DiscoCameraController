
find_package(Vmb REQUIRED COMPONENTS CPP)

add_library(libproviders STATIC VimbaProvider.cpp)
target_include_directories(libproviders PRIVATE ${CMAKE_SOURCE_DIR}/src/include/providers)

target_link_libraries(libproviders PRIVATE Vmb::CPP)