cmake_minimum_required(VERSION 3.10)
project(root)

#set( CMAKE_VERBOSE_MAKEFILE on )

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_COMPILER_CLANG true)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_COMPILER_GCC true)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_COMPILER_INTEL true)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_COMPILER_MSVC true)
endif()

function(wasp_target_set_cxx_standard TARGET_NAME)
    # MSVC requires `/std:c++latest` for various features in C++17 (e.g. dot struct initializers)
    if (CMAKE_CXX_COMPILER_MSVC)
        if (MSVC_VERSION GREATER_EQUAL 1900)
            target_compile_options(${TARGET_NAME} PRIVATE "/std:c++latest")
        else()
            message(FATAL_ERROR "MSVC version ${MSVC_VERSION} does not support C++17. Expected MSVC_VERSION >= 1900 (VS2015+)")
        endif()
    else()
        set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
    endif()
endfunction()

function(wasp_target_link_backend_library TARGET_NAME)
    # see also
    # > https://stackoverflow.com/a/42083877/809572
    # > https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
    # > https://cmake.org/cmake/help/latest/command/target_link_libraries.html#id2 (under "a link flag")
    #

    if (CMAKE_CXX_COMPILER_MSVC)
        set(WASP_BACKEND_TARGET_NAME wasp_backend_windows)
        target_link_libraries(${TARGET_NAME} PRIVATE -wholearchive:$<TARGET_LINKER_FILE:wasp_backend_windows>)

    elseif(LINUX)
        set(WASP_BACKEND_TARGET_NAME wasp_backend_linux)

        if (CMAKE_CXX_COMPILER_GCC)
            # see also https://stackoverflow.com/a/50281571/809572
            target_link_libraries(${TARGET_NAME} PRIVATE "-Wl,--whole-archive $<TARGET_LINKER_FILE:wasp_backend_linux> -Wl,--no-whole-archive")

        elseif (CMAKE_CXX_COMPILER_CLANG)
            target_link_libraries(${TARGET_NAME} PRIVATE "-Wl,-force_load $<TARGET_LINKER_FILE:wasp_backend_linux>")

        else()
            message(FATAL_ERROR "Unsupported Linux compiler '${CMAKE_CXX_COMPILER_ID}' while attempting to add WASP backend to ${TARGET_NAME}.")
        endif()

    else()
        message(FATAL_ERROR "Unsupported platform and/or compiler while attempting to add WASP backend to ${TARGET_NAME}.")
    endif()

    add_dependencies(${TARGET_NAME} ${WASP_BACKEND_TARGET_NAME})
    get_target_property(WASP_BACKEND_INTERFACE_LIBRARIES ${WASP_BACKEND_TARGET_NAME} INTERFACE_LINK_LIBRARIES)
    target_link_libraries(${TARGET_NAME} PRIVATE ${WASP_BACKEND_INTERFACE_LIBRARIES})

endfunction()

function(wasp_target_link_libraries TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE wasp)
    wasp_target_link_backend_library(${TARGET_NAME})
endfunction()

macro(wasp_get_dirname NAME)
    # see also https://stackoverflow.com/a/12345018/809572
    get_filename_component(${NAME} ${CMAKE_CURRENT_LIST_DIR} NAME)
    string(REPLACE " " "_" ${NAME} ${${NAME}})
endmacro()

###

add_subdirectory(core)
add_subdirectory(core_wrapper)
add_subdirectory(platform)
add_subdirectory(demos)
