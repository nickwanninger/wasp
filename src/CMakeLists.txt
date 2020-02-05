cmake_minimum_required(VERSION 3.10)
project(mobo CXX)

add_subdirectory(platform)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(mobo "")

target_sources(mobo
        PRIVATE
        main.cpp
        vcpu.cpp
        machine.cpp
        ${MOBO_PLATFORM_SOURCES}
        )

set_property(TARGET mobo PROPERTY CXX_STANDARD 17)

target_include_directories(mobo
        PUBLIC ../include)

target_link_libraries(mobo Threads::Threads)
