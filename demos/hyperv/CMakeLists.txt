cmake_minimum_required(VERSION 3.10)
project(hyperv_demo CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Unsupported platform. Did you forgot to check the platform before including this folder?")
endif()

add_executable(${PROJECT_NAME} "")

if (MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_options(${PROJECT_NAME} PRIVATE "/std:c++latest")
else()
    message(FATAL_ERROR "MSVC version ${MSVC_VERSION} does not support C++17. Expected MSVC_VERSION >= 1900 (VS2015+)")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "kernel.elf")

target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        _AMD64_
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        winhvplatform.lib  # Hyper-V
        mincore.lib # VirtualAlloc2()
        )

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ../../include
        )

target_sources(hyperv_demo
        PRIVATE
        hyperv_demo.cpp
        ../../core/vcpu.cpp
        ../../core/machine.cpp
        ../../core/platform.cpp
        ../../platform/windows/hyperv_machine.cpp
        ../../platform/windows/hyperv_vcpu.cpp
        )
