cmake_minimum_required(VERSION 3.10)
project(root)

#set( CMAKE_VERBOSE_MAKEFILE on )

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

add_subdirectory(src)
#add_subdirectory(kernel/src)
