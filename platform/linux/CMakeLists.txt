cmake_minimum_required(VERSION 3.10)
project(wasp_backend_linux CXX)

if (NOT LINUX)
    message(FATAL_ERROR "Unsupported platform. Did you forgot to check the platform before including this folder?")
endif()

set(LIBRARY_NAME ${PROJECT_NAME}_obj)
add_library(${LIBRARY_NAME} OBJECT "")
set_target_properties(${LIBRARY_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(${LIBRARY_NAME} PRIVATE WASP_LIBRARY_EXPORT)

wasp_target_set_lang_standard(${LIBRARY_NAME})

target_include_directories(${LIBRARY_NAME}
        PUBLIC ../../include)

target_link_libraries(${LIBRARY_NAME}
        PUBLIC
        wasp)

target_sources(${LIBRARY_NAME}
        PRIVATE
        platform.cpp
        kvm.cpp
        )

add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${LIBRARY_NAME}>)
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

add_library(${PROJECT_NAME}_dyn SHARED $<TARGET_OBJECTS:${LIBRARY_NAME}>)
set_target_properties(${PROJECT_NAME}_dyn PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_dyn")
