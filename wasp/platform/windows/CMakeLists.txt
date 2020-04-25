cmake_minimum_required(VERSION 3.10)
project(wasp_backend_windows CXX)

if (NOT WIN32)
    message(FATAL_ERROR "Unsupported platform. Did you forgot to check the platform before including this folder?")
endif()

set(LIBRARY_NAME ${PROJECT_NAME})
add_library(${LIBRARY_NAME} STATIC "")

target_include_directories(${LIBRARY_NAME}
        PUBLIC ../../include)

wasp_target_set_lang_standard(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE
        platform.cpp
        getopt.c
        socket.cpp
        hyperv_vcpu.cpp
        hyperv_machine.cpp
        )

target_compile_definitions(${LIBRARY_NAME}
        PRIVATE
        _AMD64_)

target_link_libraries(${LIBRARY_NAME}
        PUBLIC
        wasp
        winhvplatform.lib  # Hyper-V
        ws2_32.lib  # WinSock 2
        mincore.lib  # support `VirtualAlloc2`
        )
