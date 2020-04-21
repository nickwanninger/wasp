cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory(windows)
elseif(LINUX)
    add_subdirectory(linux)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
