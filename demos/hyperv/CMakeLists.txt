cmake_minimum_required(VERSION 3.10)
project(hyperv_demo CXX)

# MSVC requires `/std:c++latest` for various features in C++17 (e.g. dot struct initializers)
if (MSVC)
    add_executable(hyperv_demo "")

    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_compile_options(hyperv_demo PRIVATE "/std:c++latest")
    else()
        message(FATAL_ERROR "MSVC version ${MSVC_VERSION} does not support C++17. Expected MSVC_VERSION >= 1900 (VS2015+)")
    endif()

    set_target_properties(hyperv_demo PROPERTIES
            VS_DEBUGGER_COMMAND_ARGUMENTS "kernel.elf")

    target_compile_definitions(hyperv_demo
            PRIVATE
            _AMD64_
            )

    target_link_libraries(hyperv_demo
            PRIVATE
            winhvplatform.lib  # Hyper-V
            mincore.lib # VirtualAlloc2()
            )

    target_include_directories(hyperv_demo
            PRIVATE
            ../../include
            )

    target_sources(hyperv_demo
            PRIVATE
            hyperv_demo.cpp
            ../../core/vcpu.cpp
            ../../core/machine.cpp
            ../../platform/windows/hyperv_machine.cpp
            ../../platform/windows/hyperv_vcpu.cpp
            )

endif()
