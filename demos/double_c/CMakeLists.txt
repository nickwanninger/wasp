cmake_minimum_required(VERSION 3.10)

wasp_get_dirname(DEMO_NAME)
project(demo_${DEMO_NAME} C)
wasp_target_configure_demo(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE
        main.c
        runner.c
        workload.c)
