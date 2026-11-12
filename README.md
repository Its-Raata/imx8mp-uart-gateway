# i.MX8MP UART Gateway

**Author:** Raata <its.raata@gmail.com>  
**Language:** C
**Build System:** CMake
**Target Platform:** NXP i.MX8M Plus
**Development Environment:** Ubuntu Linux

## Overview

The i.MX8MP UART Gateway is an Embedded C project focused on building reliable UART communication between a Linux-based embedded system and an external microcontroller or serial device.

The project is being developed incrementally, starting with basic UART transmission and reception and progressing toward a robust binary communication protocol.

The main objectives are:

- Reliable UART communication using Linux system interfaces.
- Robust handling of UART byte streams and message boundaries.
- A transport-independent binary protocol parser.
- Frame validation using CRC32.
- Automated unit testing and error-handling verification.
- Integration into an embedded Linux system using Yocto.

## Project Architecture

The project is organized into independent components:

```text
                 Linux Application
                        |
              UART Communication
                 (TX / RX)
                        |
                  Binary Protocol
                     Parser
                        |
                  UART Transport
                        |
                 External Device
```

The binary protocol parser is designed to be independent of the underlying transport. It processes arbitrary byte sequences and does not directly access UART devices.

This allows the same parser to be tested independently and later reused with different communication interfaces.

## Current Implementation

### Step 01 — Basic UART Communication

**Status: Completed**

Implemented:

- UART transmission and reception using Linux `termios`.
- Configurable serial communication parameters.
- 115200 baud, 8 data bits, no parity, 1 stop bit (8N1).
- Reliable transmission using buffered writes.
- Buffered reception with support for fragmented messages.
- Handling of multiple messages received in a single read.
- Oversized-message detection and recovery.

**Hardware validation:**

UART transmission from the i.MX8MP to an Ubuntu-based receiver was successfully tested using a USB-to-UART connection.

Raw received bytes were checked to verify data integrity.

### Step 02 — Binary UART Protocol

**Status: Implemented and unit-tested on Ubuntu**

A binary frame format and incremental parser have been implemented.

#### Frame Format

| Field | Size | Description |
|---|---:|---|
| SOF | 2 bytes | Frame synchronization (`AA 55`) |
| Version | 1 byte | Protocol version |
| Message Type | 1 byte | Message identifier |
| Sequence ID | 2 bytes | Big-endian sequence number |
| Payload Length | 2 bytes | Big-endian payload length |
| Payload | 0–256 bytes | Message data |
| CRC32 | 4 bytes | Frame integrity check |

Maximum frame size: **268 bytes**.

#### CRC32

The protocol uses CRC-32/ISO-HDLC (IEEE CRC32).

- Reflected polynomial: `0xEDB88320`
- Initial value: `0xFFFFFFFF`
- Final XOR: `0xFFFFFFFF`
- Coverage: Version through the end of the payload.
- CRC field encoding: Big-endian.

#### Parser Capabilities

- Incremental parsing of incoming byte streams.
- Handling of fragmented frames.
- Handling of multiple concatenated frames.
- SOF detection and synchronization.
- Protocol version validation.
- Payload length validation.
- CRC32 validation.
- Recovery after invalid frames.
- Handling of corrupted frames containing another valid frame.
- Support for empty and maximum-sized payloads.

The parser is implemented as a standalone C module and does not depend on UART hardware or Linux serial-device interfaces.

#### Unit Testing

The protocol implementation currently has 11 unit tests covering:

1. Standard CRC32 verification.
2. Fragmented frame reception.
3. Multiple frames in one input buffer.
4. Empty payload.
5. Maximum-sized payload.
6. Invalid CRC handling.
7. Oversized payload rejection.
8. Unsupported protocol version.
9. Garbage bytes and overlapping SOF candidates.
10. Valid nested frame inside a valid outer frame.
11. Recovery of a valid nested frame after outer-frame corruption.

All 11 tests pass in both the normal build and AddressSanitizer/UndefinedBehaviorSanitizer builds.

## Repository Structure

```text
imx8mp-uart-gateway/
├── CMakeLists.txt
├── .gitignore
│
├── docs/
│   ├── README.md
│   ├── development-plan.md
│   └── protocol.md
│
├── include/
│   └── uart_protocol.h
│
├── src/
│   ├── uart_tx.c
│   ├── uart_rx.c
│   └── uart_protocol.c
│
└── tests/
    ├── uart_test.py
    ├── uart_test2.py
    └── test_uart_protocol.c
```

## Build and Test

### Requirements

- GCC
- CMake 3.16 or newer
- Linux development environment

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build
```

### Run Unit Tests

```bash
ctest --test-dir build --output-on-failure
```

Run the protocol test executable directly:

```bash
./build/test_uart_protocol
```

### Build with AddressSanitizer and UndefinedBehaviorSanitizer

```bash
cmake -S . -B build-sanitize \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build build-sanitize
```

Run the sanitizer tests:

```bash
ctest --test-dir build-sanitize --output-on-failure
```

## Development Roadmap

The project is being developed in the following stages:

| Step | Description | Status |
|---|---|---|
| 01 | Basic UART transmission and reception | Completed |
| 02 | Binary protocol and incremental parser | Implemented and unit-tested |
| 03 | Non-blocking I/O, request/response handling, timeouts and retries | Planned |
| 04 | External microcontroller integration and hardware validation | Planned |
| 05 | Yocto integration, cross-compilation and systemd service | Planned |
| 06 | Reliability, stress testing and performance measurements | Planned |

## Design Principles

The project follows several design principles:

- **Modularity:** Keep protocol handling independent of transport implementation.
- **Portability:** Use standard C and Linux interfaces where appropriate.
- **Reliability:** Validate incoming data and recover from malformed frames.
- **Testability:** Develop and verify protocol functionality independently of hardware.
- **Maintainability:** Keep source code, public interfaces, tests and documentation organized.

## Author

**Raata**

GitHub: [Its-Raata](https://github.com/Its-Raata)