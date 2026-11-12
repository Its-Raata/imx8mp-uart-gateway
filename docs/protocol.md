## Author

Raata <its.raata@gmail.com>

# UART Binary Protocol Specification

## 1. Overview

This document defines the binary communication protocol used by
the i.MX8MP UART Gateway.

The protocol is independent of the physical transport.

Protocol version: 1

## 2. Frame Structure

| Field          | Size       | Description                  |
|----------------|------------|------------------------------|
| SOF            | 2 bytes    | Start of frame: AA 55        |
| Version        | 1 byte     | Protocol version             |
| Message Type   | 1 byte     | Message identifier            |
| Sequence ID    | 2 bytes    | Request/response identifier  |
| Payload Length | 2 bytes    | Payload length in bytes      |
| Payload        | 0-256      | Message data                 |
| CRC32          | 4 bytes    | Frame integrity check        |

Header size: 8 bytes

Maximum payload: 256 bytes

CRC size: 4 bytes

Maximum frame size: 268 bytes

## 3. Byte Order

All multi-byte integer fields use Big-endian byte order.

Examples:

Sequence ID 0x1234:
12 34

Payload length 16:
00 10

CRC32 is also transmitted in Big-endian byte order.

## 4. Start of Frame

The start marker is:

AA 55

The parser searches for this sequence to synchronize with the stream.

## 5. Protocol Version

The initial protocol version is:

01

## 6. Message Type

Message Type identifies the purpose of a frame.

Initial proposed message types:

0x01 - PING request
0x02 - PING response
0x10 - DATA request
0x11 - DATA response
0x7F - ERROR response

These identifiers can be extended later.

## 7. Sequence ID

Sequence ID is an unsigned 16-bit integer.

It is used to associate responses with requests.

The value wraps around after 0xFFFF.

## 8. Payload Length

Payload Length is an unsigned 16-bit integer.

The maximum accepted payload is 256 bytes.

A frame declaring a larger payload must be rejected.

## 9. CRC32

Algorithm: CRC-32/ISO-HDLC (CRC-32/IEEE)

Parameters:

Polynomial:       0x04C11DB7
Reflected input:  Yes
Reflected output: Yes
Initial value:    0xFFFFFFFF
Final XOR:        0xFFFFFFFF

The CRC is calculated over:

Version + Message Type + Sequence ID + Payload Length + Payload

SOF and CRC fields are excluded.

## 10. Parser Requirements

The parser must:

- Accept fragmented frames across multiple read operations.
- Accept multiple frames in one input buffer.
- Search for the SOF marker.
- Reject payload lengths greater than 256 bytes.
- Reject frames with invalid CRC.
- Recover synchronization after corrupted data.
- Avoid depending on UART device configuration.

## 11. Example

Message:

Version:       01
Message Type:  10
Sequence ID:   0001
Payload:       HELLO

Frame before CRC:

AA 55
01
10
00 01
00 05
48 45 4C 4C 4F

The CRC32 is appended as four bytes in Big-endian order.

## 12. Implementation and Testing

The protocol implementation is written in C11.

The parser is independent of the UART transport and can be tested
on Ubuntu without physical hardware.

Build and run the tests:

    cmake -S . -B build
    cmake --build build
    ctest --test-dir build --output-on-failure

Author: Raata <its.raata@gmail.com>
