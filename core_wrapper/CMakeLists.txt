cmake_minimum_required(VERSION 3.10)
project(wasp_c CXX)

add_library(${PROJECT_NAME} STATIC "")

target_include_directories(${PROJECT_NAME}
        PUBLIC ../include)

wasp_target_set_cxx_standard(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        wasp)

target_sources(${PROJECT_NAME}
        PRIVATE
        machine.cpp
        loader.cpp
        )

