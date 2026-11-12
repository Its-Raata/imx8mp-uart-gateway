## Author

Raata <its.raata@gmail.com>

# i.MX8MP UART Gateway

A practical Embedded Linux project focused on reliable UART
communication between an i.MX8MP Linux system and an external device.

## Project goals

- UART transmission and reception on Embedded Linux.
- A robust binary communication protocol.
- Reliable frame parsing and error detection.
- Asynchronous communication and request/response handling.
- Integration into Yocto Linux using a systemd service.
- Reliability and stress testing.

## Development stages

### Step 01 - Basic UART Communication [COMPLETED]

- UART transmitter.
- UART receiver.
- Serial configuration using termios.
- Buffered line reception.
- Fragmented and concatenated message handling.
- Oversized-message detection and recovery.

### Step 02 - Binary UART Protocol [IN PROGRESS]

- Define binary frame format.
- Implement CRC32.
- Implement stream-based frame parser.
- Handle fragmented and concatenated frames.
- Handle corrupted frames and synchronization recovery.

### Step 03 - Asynchronous Communication

- Nonblocking UART I/O.
- poll()/epoll().
- Request/response mechanism.
- Timeouts and retries.

### Step 04 - External Device Integration

- Implement communication with an external MCU.
- Test real hardware communication.

### Step 05 - Yocto Integration

- Create a Yocto recipe.
- Integrate a systemd service.

### Step 06 - Reliability Testing

- Stress testing.
- Error injection.
- Performance measurements.


## Build and Test (Ubuntu)

Requirements:
- GCC
- CMake 3.16 or newer

Configure the project:

    cmake -S . -B build

Build all targets:

    cmake --build build

Run unit tests:

    ctest --test-dir build --output-on-failure

Run the protocol test executable directly:

    ./build/test_uart_protocol

Clean the build directory:

    cmake --build build --target clean

Remove the complete build directory:

    rm -rf build
