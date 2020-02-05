cmake_minimum_required(VERSION 3.10)

if (WIN32)
    set(MOBO_PLATFORM_SOURCES
            windows/platform.cpp
            windows/getopt.c
            windows/hyperv.cpp
            )

elseif(LINUX)
    set(MOBO_PLATFORM_SOURCES
            linux/platform.cpp
            linux/kvm.cpp
            )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
