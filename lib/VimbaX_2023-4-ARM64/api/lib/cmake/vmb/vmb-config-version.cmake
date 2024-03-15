set(PACKAGE_ARCHITECTURE_SUITABLE 0)

if (APPLE AND CMAKE_OSX_ARCHITECTURES STREQUAL "")
    set(PACKAGE_ARCHITECTURE_SUITABLE 1)
    if(DEFINED CMAKE_SYSTEM_PROCESSOR AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "" AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(PACKAGE_ARCHITECTURE_SUITABLE 0)
    endif()
elseif (NOT CMAKE_LIBRARY_ARCHITECTURE)
    # message(WARNING "CMAKE_LIBRARY_ARCHITECTURE not set, cannot check platform compatibility")
    set(PACKAGE_ARCHITECTURE_SUITABLE 1)
else ()
    if (UNIX)
        if (CMAKE_LIBRARY_ARCHITECTURE STREQUAL "aarch64-linux-gnu")
            set(PACKAGE_ARCHITECTURE_SUITABLE 1)
            if(DEFINED CMAKE_SYSTEM_PROCESSOR AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "" AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
                set(PACKAGE_ARCHITECTURE_SUITABLE 0)
            endif()
        endif()
    elseif(WIN32)
        if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
            set(PACKAGE_ARCHITECTURE_SUITABLE 1)
        endif()
    else()
        message("Unsupported os ${CMAKE_SYSTEM_NAME")
    endif()
endif()

# if (NOT PACKAGE_ARCHITECTURE_SUITABLE OR NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
#     set(PACKAGE_VERSION_UNSUITABLE True)
# else()
#     set(PACKAGE_VERSION "1.0.5")

#     if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
#       set(PACKAGE_VERSION_COMPATIBLE FALSE)
#     else()
#       set(PACKAGE_VERSION_COMPATIBLE TRUE)
#       if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
#         set(PACKAGE_VERSION_EXACT TRUE)
#       endif()
#     endif()
# endif()
set(PACKAGE_VERSION "1.0.5")
set(PACKAGE_VERSION_COMPATIBLE TRUE)
set(PACKAGE_ARCHITECTURE_SUITABLE 1)