cmake_minimum_required(VERSION 3.10)
project(mobo CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(mobo "")

# MSVC requires `/std:c++latest` for various features in C++17 (e.g. dot struct initializers)
if (MSVC)
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_compile_options(mobo PRIVATE "/std:c++latest")
    else()
        message(FATAL_ERROR "MSVC version ${MSVC_VERSION} does not support C++17. Expected MSVC_VERSION >= 1900 (VS2015+)")
    endif()

    set_target_properties(mobo PROPERTIES
            VS_DEBUGGER_COMMAND_ARGUMENTS "kernel.elf")
else()
    set_property(TARGET mobo PROPERTY CXX_STANDARD 17)
endif()

target_include_directories(mobo
        PUBLIC ../include)

target_link_libraries(mobo
        PRIVATE Threads::Threads)

target_sources(mobo
        PRIVATE
        main.cpp
        vcpu.cpp
        machine.cpp
        platform.cpp

        loader/elf_loader.cpp
        loader/flatbin_loader.cpp

        workload/boottime_workload.cpp
        workload/double_workload.cpp
        workload/fib_workload.cpp
        workload/tcp_workload.cpp
        )

if (WIN32)
    target_sources(mobo
            PRIVATE
            platform/windows/platform.cpp
            platform/windows/getopt.c
            platform/windows/socket.cpp
            platform/windows/hyperv_vcpu.cpp
            platform/windows/hyperv_machine.cpp
            )

    target_compile_definitions(mobo
            PRIVATE
            _AMD64_)

    target_link_libraries(mobo
            PRIVATE
            winhvplatform.lib  # Hyper-V
            ws2_32.lib  # WinSock 2
            mincore.lib  # support `VirtualAlloc2`
            )

elseif(LINUX)
    target_sources(mobo
            PRIVATE
            platform/linux/platform.cpp
            platform/linux/kvm.cpp
            )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

