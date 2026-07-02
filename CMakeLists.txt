cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rocket_logger C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rocket_logger
    main.c
    imu_driver.c
    sd_logger.c
)

target_link_libraries(rocket_logger
    pico_stdlib
    hardware_i2c
    hardware_spi
    hardware_gpio
)

pico_enable_stdio_usb(rocket_logger 1)
pico_enable_stdio_uart(rocket_logger 0)

pico_add_extra_outputs(rocket_logger)
