cmake_minimum_required(VERSION 3.10)
project(mobo CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(mobo "")

set(MOBO_SOURCES
        main.cpp
        vcpu.cpp
        kvm.cpp
        machine.cpp)

if (WIN32)
    set(MOBO_SOURCES ${MOBO_SOURCES} platform/windows/platform.cpp)
elseif(LINUX)
    set(MOBO_SOURCES ${MOBO_SOURCES} platform/linux/platform.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_sources(mobo
        PRIVATE ${MOBO_SOURCES})

set_property(TARGET mobo PROPERTY CXX_STANDARD 17)

target_include_directories(mobo
        PUBLIC ../include)

target_link_libraries(mobo Threads::Threads)
