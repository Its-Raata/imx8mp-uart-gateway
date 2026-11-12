# Author: Raata <its.raata@gmail.com>

cmake_minimum_required(VERSION 3.16)

project(imx8mp_uart_gateway
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# Step 01: UART applications
add_executable(uart_tx src/uart_tx.c)
add_executable(uart_rx src/uart_rx.c)

# uart_tx.c already defines _DEFAULT_SOURCE.
target_compile_definitions(uart_rx PRIVATE _DEFAULT_SOURCE)

# Step 02: Protocol library
add_library(uart_protocol STATIC
    src/uart_protocol.c
)

target_include_directories(uart_protocol
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Step 02: Unit tests
enable_testing()

add_executable(test_uart_protocol
    tests/test_uart_protocol.c
)

target_link_libraries(test_uart_protocol
    PRIVATE uart_protocol
)

add_test(
    NAME uart_protocol_tests
    COMMAND test_uart_protocol
)