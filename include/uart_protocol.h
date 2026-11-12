/*
 * Project: i.MX8MP UART Gateway
 * Author: Raata <its.raata@gmail.com>
 * Step: 02 - Binary UART Protocol
 */

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define UART_SOF_1               0xAA
#define UART_SOF_2               0x55

#define UART_PROTOCOL_VERSION    0x01

#define UART_HEADER_SIZE         8
#define UART_CRC_SIZE            4

#define UART_MAX_PAYLOAD_SIZE    256
#define UART_MAX_FRAME_SIZE      \
    (UART_HEADER_SIZE + UART_MAX_PAYLOAD_SIZE + UART_CRC_SIZE)

#define UART_PARSER_BUFFER_SIZE  (UART_MAX_FRAME_SIZE * 2)

typedef struct
{
    uint8_t version;
    uint8_t message_type;
    uint16_t sequence_id;
    uint16_t payload_length;
    uint8_t payload[UART_MAX_PAYLOAD_SIZE];

} uart_frame_t;

typedef struct
{
    uint8_t buffer[UART_PARSER_BUFFER_SIZE];
    size_t length;

} uart_parser_t;

typedef void (*uart_frame_callback_t)(const uart_frame_t *frame,
                                      void *user_data);

void uart_parser_init(uart_parser_t *parser);

void uart_parser_feed(uart_parser_t *parser,
                      const uint8_t *data,
                      size_t length,
                      uart_frame_callback_t callback,
                      void *user_data);

uint32_t uart_crc32(const uint8_t *data, size_t length);

#endif
