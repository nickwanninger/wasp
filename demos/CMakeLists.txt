cmake_minimum_required(VERSION 3.10)

function(wasp_target_configure_demo PROJECT_NAME)
    add_executable(${PROJECT_NAME} "")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${DEMO_NAME}")

		target_link_libraries(${PROJECT_NAME} wasp)
    target_compile_definitions(${PROJECT_NAME}
            PRIVATE
            _AMD64_
            )

    target_include_directories(${PROJECT_NAME}
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include)
endfunction()

# add_subdirectory(boottime)
# add_subdirectory(double)
# add_subdirectory(double_c)
# add_subdirectory(fib)
# add_subdirectory(http_echo)


add_subdirectory(test)

if (WIN32)
    add_subdirectory(hyperv)
endif()
