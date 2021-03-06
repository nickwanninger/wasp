cmake_minimum_required(VERSION 3.10)
project(wasp CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(LIBRARY_NAME ${PROJECT_NAME})
add_library(${LIBRARY_NAME} SHARED "")

install(TARGETS ${LIBRARY_NAME})

target_include_directories(${LIBRARY_NAME}
        PUBLIC ../include)

target_link_libraries(${LIBRARY_NAME}
        PRIVATE Threads::Threads)

wasp_target_set_lang_flags(${LIBRARY_NAME})

if (CMAKE_CXX_COMPILER_CLANG OR CMAKE_CXX_COMPILER_GCC)
# Enable Position-Independent-Code when building static libraries for Rust
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

target_sources(${LIBRARY_NAME}
        PRIVATE
        vcpu.cpp
        machine.cpp
        runner.cpp
        loader/elf_loader.cpp
        loader/flatbin_loader.cpp
				c_wrapper/loader.cpp
				c_wrapper/machine.cpp
)


if (WIN32)
target_sources(${LIBRARY_NAME}
	PRIVATE
	platform/windows/getopt.c
	platform/windows/hyperv_machine.cpp
	platform/windows/hyperv_vcpu.cpp
	platform/windows/platform.cpp
	platform/windows/socket.cpp
)
endif(WIN32)




if (LINUX)
target_sources(${LIBRARY_NAME}
	PRIVATE
	platform/linux/kvm.cpp
	platform/linux/platform.cpp
)
endif(LINUX)
