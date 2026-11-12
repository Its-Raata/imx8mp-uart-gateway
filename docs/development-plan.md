## Author

Raata <its.raata@gmail.com>

# Development Plan

## Architecture

The project separates UART transport from the communication protocol.

UART transport is responsible for:
- Opening and configuring the serial device.
- Reading and writing bytes.
- Handling serial I/O errors.

The protocol layer is responsible for:
- Constructing binary frames.
- Parsing incoming byte streams.
- Validating frame structure.
- Checking CRC32.
- Detecting and recovering from corrupted frames.

The protocol layer must not depend on a specific UART device.

## Coding principles

- Use standard C and POSIX APIs.
- Keep modules small and independent.
- Check return values and handle errors.
- Avoid unnecessary dynamic memory allocation.
- Test protocol functionality independently of hardware.
- Use compiler warnings during development.

## Step 02 implementation sequence

1. Define protocol constants and data structures.
2. Implement CRC32.
3. Implement the incremental frame parser.
4. Test fragmented frames.
5. Test multiple frames in one input buffer.
6. Test invalid CRC and synchronization recovery.
7. Review the implementation line by line.

## Ubuntu Development Commands

Configure:

    cmake -S . -B build

Build:

    cmake --build build

Test:

    ctest --test-dir build --output-on-failure

The protocol unit tests run entirely on Ubuntu.
They do not require an i.MX8MP board or a UART connection.
