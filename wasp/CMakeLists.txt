cmake_minimum_required(VERSION 3.10)
project(wasp CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(LIBRARY_NAME ${PROJECT_NAME})
add_library(${LIBRARY_NAME} STATIC "")

target_include_directories(${LIBRARY_NAME}
        PUBLIC ../include)

target_link_libraries(${LIBRARY_NAME}
        PRIVATE Threads::Threads)

#wasp_target_set_lang_standard(${LIBRARY_NAME})

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
