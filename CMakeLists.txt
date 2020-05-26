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


function(wasp_target_set_lang_flags TARGET_NAME)
    get_property(PROJECT_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)

    if ("C" IN_LIST PROJECT_LANGUAGES)
        set_property(TARGET ${TARGET_NAME} PROPERTY C_STANDARD 11)
    endif()

    if ("CXX" IN_LIST PROJECT_LANGUAGES)
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
    endif()

endfunction()


macro(wasp_get_dirname NAME)
    # see also https://stackoverflow.com/a/12345018/809572
    get_filename_component(${NAME} ${CMAKE_CURRENT_LIST_DIR} NAME)
    string(REPLACE " " "_" ${NAME} ${${NAME}})
endmacro()

###

add_subdirectory(wasp)
# add_subdirectory(demos)
