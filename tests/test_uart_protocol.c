/*
 * Project: i.MX8MP UART Gateway
 * Author: Raata <its.raata@gmail.com>
 * Step: 02 - Binary UART Protocol
 */

#include "uart_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_TEST_FRAMES 20

typedef struct
{
    uart_frame_t frames[MAX_TEST_FRAMES];
    size_t count;

} test_context_t;

static void receive_frame(const uart_frame_t *frame, void *user_data)
{
    test_context_t *context = user_data;

    assert(context->count < MAX_TEST_FRAMES);

    context->frames[context->count] = *frame;
    context->count++;
}

static size_t build_frame(uint8_t *output,
                          uint8_t version,
                          uint8_t message_type,
                          uint16_t sequence_id,
                          const uint8_t *payload,
                          uint16_t payload_length)
{
    output[0] = UART_SOF_1;
    output[1] = UART_SOF_2;
    output[2] = version;
    output[3] = message_type;

    output[4] = (uint8_t)(sequence_id >> 8);
    output[5] = (uint8_t)sequence_id;

    output[6] = (uint8_t)(payload_length >> 8);
    output[7] = (uint8_t)payload_length;

    if (payload_length > 0)
    {
        memcpy(&output[8], payload, payload_length);
    }

    uint32_t crc = uart_crc32(&output[2],
                              6 + payload_length);

    size_t crc_position = 8 + payload_length;

    output[crc_position]     = (uint8_t)(crc >> 24);
    output[crc_position + 1] = (uint8_t)(crc >> 16);
    output[crc_position + 2] = (uint8_t)(crc >> 8);
    output[crc_position + 3] = (uint8_t)crc;

    return crc_position + 4;
}

int main(void)
{
    uart_parser_t parser;
    test_context_t context = {0};

    uart_parser_init(&parser);

    /* Test 1: Standard CRC32 test vector. */

    assert(uart_crc32((const uint8_t *)"123456789", 9)
           == 0xCBF43926);

    /* Test 2: Fragmented frame, including split SOF. */

    uint8_t frame[UART_MAX_FRAME_SIZE];
    const uint8_t hello[] = "HELLO";

    size_t frame_length =
        build_frame(frame, 1, 0x10, 1,
                    hello, 5);

    uart_parser_feed(&parser, frame, 1,
                     receive_frame, &context);

    uart_parser_feed(&parser, frame + 1, 2,
                     receive_frame, &context);

    uart_parser_feed(&parser, frame + 3, frame_length - 3,
                     receive_frame, &context);

    assert(context.count == 1);
    assert(context.frames[0].sequence_id == 1);
    assert(context.frames[0].payload_length == 5);
    assert(memcmp(context.frames[0].payload, "HELLO", 5) == 0);

    /* Test 3: Multiple frames in one input buffer. */

    uint8_t frame2[UART_MAX_FRAME_SIZE];
    uint8_t frame3[UART_MAX_FRAME_SIZE];
    uint8_t combined[UART_MAX_FRAME_SIZE * 2];

    size_t len2 = build_frame(frame2, 1, 0x10, 2,
                              (const uint8_t *)"WORLD", 5);

    size_t len3 = build_frame(frame3, 1, 0x10, 3,
                              (const uint8_t *)"TEST", 4);

    memcpy(combined, frame2, len2);
    memcpy(combined + len2, frame3, len3);

    uart_parser_feed(&parser, combined, len2 + len3,
                     receive_frame, &context);

    assert(context.count == 3);
    assert(context.frames[1].sequence_id == 2);
    assert(context.frames[2].sequence_id == 3);

    /* Test 4: Empty payload. */

    uint8_t empty_frame[UART_MAX_FRAME_SIZE];

    size_t empty_length =
        build_frame(empty_frame, 1, 0x10, 4, NULL, 0);

    uart_parser_feed(&parser, empty_frame, empty_length,
                     receive_frame, &context);

    assert(context.count == 4);
    assert(context.frames[3].payload_length == 0);

    /* Test 5: Maximum-size payload. */

    uint8_t max_payload[UART_MAX_PAYLOAD_SIZE];

    for (size_t i = 0; i < sizeof(max_payload); i++)
    {
        max_payload[i] = (uint8_t)i;
    }

    uint8_t max_frame[UART_MAX_FRAME_SIZE];

    size_t max_length =
        build_frame(max_frame, 1, 0x10, 5,
                    max_payload, sizeof(max_payload));

    uart_parser_feed(&parser, max_frame, max_length,
                     receive_frame, &context);

    assert(context.count == 5);
    assert(context.frames[4].payload_length == UART_MAX_PAYLOAD_SIZE);
    assert(memcmp(context.frames[4].payload,
                  max_payload, sizeof(max_payload)) == 0);

    /* Test 6: Invalid CRC followed by a valid frame. */

    uint8_t bad_frame[UART_MAX_FRAME_SIZE];

    size_t bad_length =
        build_frame(bad_frame, 1, 0x10, 6,
                    hello, 5);

    bad_frame[8] ^= 0x01;

    uart_parser_feed(&parser, bad_frame, bad_length,
                     receive_frame, &context);

    assert(context.count == 5);

    uart_parser_feed(&parser, frame3, len3,
                     receive_frame, &context);

    assert(context.count == 6);
    assert(context.frames[5].sequence_id == 3);

    /* Test 7: Oversized payload length followed by valid frame. */

    const uint8_t invalid_header[] = {
        0xAA, 0x55, 0x01, 0x10,
        0x00, 0x07, 0x01, 0x01
    };

    /* Declared payload length is 0x0101 = 257 bytes. */

    uart_parser_feed(&parser, invalid_header,
                     sizeof(invalid_header),
                     receive_frame, &context);

    uart_parser_feed(&parser, frame3, len3,
                     receive_frame, &context);

    assert(context.count == 7);
    assert(context.frames[6].sequence_id == 3);

    /* Test 8: Unsupported version followed by valid frame. */

    uint8_t wrong_version[UART_MAX_FRAME_SIZE];

    size_t wrong_length =
        build_frame(wrong_version, 2, 0x10, 7,
                    hello, 5);

    uart_parser_feed(&parser, wrong_version, wrong_length,
                     receive_frame, &context);

    assert(context.count == 7);

    uart_parser_feed(&parser, frame3, len3,
                     receive_frame, &context);

    assert(context.count == 8);
    assert(context.frames[7].sequence_id == 3);

    /* Test 9: Garbage and overlapping SOF candidates. */

    const uint8_t garbage[] = {
        0x00, 0x12, 0xAA, 0xAA, 0x55, 0x99
    };

    uart_parser_feed(&parser, garbage, sizeof(garbage),
                     receive_frame, &context);

    uart_parser_feed(&parser, frame3, len3,
                     receive_frame, &context);

    assert(context.count == 9);
    assert(context.frames[8].sequence_id == 3);

        /*
     * Test 10: A valid frame containing another valid frame.
     * Only the outer frame should be delivered.
     */

    uint8_t inner_frame[UART_MAX_FRAME_SIZE];
    size_t inner_length =
        build_frame(inner_frame, 1, 0x10, 10,
                    (const uint8_t *)"INNER", 5);

    uint8_t outer_payload[UART_MAX_PAYLOAD_SIZE];

    outer_payload[0] = 0x31;
    outer_payload[1] = 0x32;

    memcpy(&outer_payload[2], inner_frame, inner_length);

    outer_payload[2 + inner_length] = 0x33;
    outer_payload[3 + inner_length] = 0x34;

    uint16_t outer_payload_length =
        (uint16_t)(inner_length + 4);

    uint8_t outer_frame[UART_MAX_FRAME_SIZE];

    size_t outer_length =
        build_frame(outer_frame, 1, 0x10, 9,
                    outer_payload, outer_payload_length);

    uart_parser_feed(&parser, outer_frame, outer_length,
                     receive_frame, &context);

    assert(context.count == 10);
    assert(context.frames[9].sequence_id == 9);
    assert(context.frames[9].payload_length == outer_payload_length);


    /*
     * Test 11: A corrupted outer frame containing a valid inner frame.
     * The outer frame should be rejected and the inner frame recovered.
     */

    outer_frame[outer_length - 1] ^= 0x01;

    uart_parser_feed(&parser, outer_frame, outer_length,
                     receive_frame, &context);

    assert(context.count == 11);
    assert(context.frames[10].sequence_id == 10);
    assert(context.frames[10].payload_length == 5);
    assert(memcmp(context.frames[10].payload, "INNER", 5) == 0);

    printf("All 11 UART protocol tests passed.\n");

    return 0;
}