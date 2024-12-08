cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_wifi_example) 

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(HTTPRequests)

add_executable(pico_wifi_example
    main.cpp
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_wifi_example 0)
pico_enable_stdio_usb(pico_wifi_example 1)

# Add the standard include files to the build
target_include_directories(pico_wifi_example PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(pico_wifi_example
    http_requests
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    pico_lwip_http
)

pico_add_extra_outputs(pico_wifi_example)