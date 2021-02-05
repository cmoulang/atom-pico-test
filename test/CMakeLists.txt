cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(test_project)

pico_sdk_init()

add_executable(test)

pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/test.pio)

pico_enable_stdio_uart(test 1)

pico_add_extra_outputs(test)

target_sources(test PRIVATE test.c PRIVATE logic_analyser.c)

target_link_libraries(test pico_stdlib hardware_pio hardware_dma pico_multicore)


