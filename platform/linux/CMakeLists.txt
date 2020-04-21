cmake_minimum_required(VERSION 3.10)
project(wasp_backend_linux CXX)

if (NOT LINUX)
    message(FATAL_ERROR "Unsupported platform. Did you forgot to check the platform before including this folder?")
endif()

add_library(${PROJECT_NAME} STATIC "")
wasp_target_set_cxx_standard(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        PUBLIC ../../include)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        wasp)

target_sources(${PROJECT_NAME}
        PRIVATE
        platform.cpp
        kvm.cpp
        )
