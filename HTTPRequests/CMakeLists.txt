add_library(http_requests STATIC
    src/HTTPRequests.cpp
    IHTTPRequests.h
    RequestData.h
    inc/HTTPRequests.h
)

target_include_directories(http_requests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(http_requests
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    pico_lwip_http
)