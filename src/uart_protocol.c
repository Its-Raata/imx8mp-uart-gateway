/*
 * Project: i.MX8MP UART Gateway
 * Author: Raata <its.raata@gmail.com>
 * Step: 02 - Binary UART Protocol
 */

#include "uart_protocol.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *data)
{
    return ((uint16_t)data[0] << 8) |
           ((uint16_t)data[1]);
}

static uint32_t read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8)  |
           ((uint32_t)data[3]);
}

uint32_t uart_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (unsigned int bit = 0; bit < 8; bit++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF;
}

void uart_parser_init(uart_parser_t *parser)
{
    if (parser == NULL)
    {
        return;
    }

    parser->length = 0;
}

static void consume_bytes(uart_parser_t *parser, size_t count)
{
    if (count >= parser->length)
    {
        parser->length = 0;
        return;
    }

    memmove(parser->buffer,
            parser->buffer + count,
            parser->length - count);

    parser->length -= count;
}

static void process_buffer(uart_parser_t *parser,
                           uart_frame_callback_t callback,
                           void *user_data)
{
    while (1)
    {
        /*
         * Search for SOF.
         * Preserve a trailing 0xAA because it might be
         * the first byte of an SOF split across reads.
         */

        size_t pos = 0;

        while (pos + 1 < parser->length)
        {
            if (parser->buffer[pos] == UART_SOF_1 &&
                parser->buffer[pos + 1] == UART_SOF_2)
            {
                break;
            }

            pos++;
        }

        if (pos + 1 >= parser->length)
        {
            if (parser->length > 0 &&
                parser->buffer[parser->length - 1] == UART_SOF_1)
            {
                consume_bytes(parser, parser->length - 1);
            }
            else
            {
                consume_bytes(parser, parser->length);
            }

            return;
        }

        if (pos > 0)
        {
            consume_bytes(parser, pos);
        }

        /* Wait for the complete header. */

        if (parser->length < UART_HEADER_SIZE)
        {
            return;
        }

        /* Validate protocol version. */

        if (parser->buffer[2] != UART_PROTOCOL_VERSION)
        {
            consume_bytes(parser, 1);
            continue;
        }

        uint16_t payload_length =
            read_be16(&parser->buffer[6]);

        /* Reject unsupported payload lengths. */

        if (payload_length > UART_MAX_PAYLOAD_SIZE)
        {
            consume_bytes(parser, 1);
            continue;
        }

        size_t frame_length =
            UART_HEADER_SIZE + payload_length + UART_CRC_SIZE;

        /* Wait for the complete frame. */

        if (parser->length < frame_length)
        {
            return;
        }

        /* Validate CRC32. */

        uint32_t calculated_crc =
            uart_crc32(&parser->buffer[2],
                       6 + payload_length);

        uint32_t received_crc =
            read_be32(&parser->buffer[UART_HEADER_SIZE +
                                      payload_length]);

        if (calculated_crc != received_crc)
        {
            consume_bytes(parser, 1);
            continue;
        }

        /* Construct the received frame. */

        uart_frame_t frame = {0};

        frame.version = parser->buffer[2];
        frame.message_type = parser->buffer[3];
        frame.sequence_id = read_be16(&parser->buffer[4]);
        frame.payload_length = payload_length;

        if (payload_length > 0)
        {
            memcpy(frame.payload,
                   &parser->buffer[UART_HEADER_SIZE],
                   payload_length);
        }

        if (callback != NULL)
        {
            callback(&frame, user_data);
        }

        consume_bytes(parser, frame_length);
    }
}

void uart_parser_feed(uart_parser_t *parser,
                      const uint8_t *data,
                      size_t length,
                      uart_frame_callback_t callback,
                      void *user_data)
{
    if (parser == NULL || (data == NULL && length > 0))
    {
        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (parser->length >= UART_PARSER_BUFFER_SIZE)
        {
            consume_bytes(parser, 1);
        }

        parser->buffer[parser->length++] = data[i];

        process_buffer(parser, callback, user_data);
    }
}