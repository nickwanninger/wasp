cmake_minimum_required(VERSION 3.10)
project(wasp CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(LIBRARY_NAME ${PROJECT_NAME}_obj)
add_library(${LIBRARY_NAME} OBJECT "")
set_target_properties(${LIBRARY_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(${LIBRARY_NAME} PRIVATE WASP_LIBRARY_EXPORT)

target_include_directories(${LIBRARY_NAME}
        PUBLIC ../include)

target_link_libraries(${LIBRARY_NAME}
        PRIVATE Threads::Threads)

wasp_target_set_lang_standard(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE
        vcpu.cpp
        machine.cpp
        platform.cpp
        runner.cpp
        loader/elf_loader.cpp
        loader/flatbin_loader.cpp
        )

add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${LIBRARY_NAME}>)
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

add_library(${PROJECT_NAME}_dyn SHARED $<TARGET_OBJECTS:${LIBRARY_NAME}>)
set_target_properties(${PROJECT_NAME}_dyn PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_dyn")
