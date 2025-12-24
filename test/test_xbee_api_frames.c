#include "unity.h"
#include "xbee_api_frames.h"
#include "xbee_at_cmds.h"
#include "xbee.h"
#include <string.h>
#include <stdlib.h>


// ==== Global Offset for Mocks ====
static size_t mock_offset = 0;

// ==== MOCK HARDWARE FUNCTIONS ====

static int mock_uart_write(const uint8_t *data, uint16_t len){
    (void)data;
    return len;
}

static int mock_uart_read_valid(uint8_t *buffer, int len) {
    static const uint8_t fake_response[] = {
        0x7E, 0x00, 0x07, 0x88, 0x01, 'V', 'R', 0x00, 0x12, 0x6A
    };

    if (mock_offset >= sizeof(fake_response)) return 0;

    int to_copy = (mock_offset + len > sizeof(fake_response)) ?
                  (sizeof(fake_response) - mock_offset) : len;

    memcpy(buffer, &fake_response[mock_offset], to_copy);
    mock_offset += to_copy;
    return to_copy;
}

static int mock_uart_read_bad_start(uint8_t *buffer, int len) {
    (void)len;
    buffer[0] = 0x00; // Invalid delimiter
    return 1;
}

static int mock_uart_read_incomplete(uint8_t *buffer, int len) {
    static const uint8_t incomplete[] = { 0x7E, 0x00, 0x05, 0x88, 0x01, 'V' };
    if (mock_offset >= sizeof(incomplete)) return 0;

    int to_copy = (mock_offset + len > sizeof(incomplete)) ?
                  (sizeof(incomplete) - mock_offset) : len;

    memcpy(buffer, &incomplete[mock_offset], to_copy);
    mock_offset += to_copy;
    return to_copy;
}

static int mock_uart_read_bad_checksum(uint8_t *buffer, int len) {
    static const uint8_t frame[] = {
        0x7E, 0x00, 0x07, 0x88, 0x01, 'V', 'R', 0x00, 0x12, 0x00 // Invalid checksum
    };
    if (mock_offset >= sizeof(frame)) return 0;

    int to_copy = (mock_offset + len > sizeof(frame)) ?
                  (sizeof(frame) - mock_offset) : len;

    memcpy(buffer, &frame[mock_offset], to_copy);
    mock_offset += to_copy;
    return to_copy;
}

static uint32_t mock_millis(void) {
    static uint32_t t = 0;
    t += 50;
    return t;
}

static void mock_delay(uint32_t ms) {
    (void)ms;
}

// ==== MOCK TABLES ====

static XBeeHTable mock_hTable = {
    .PortUartInit  = NULL,
    .PortUartWrite = mock_uart_write,
    .PortUartRead  = mock_uart_read_valid,
    .PortMillis    = mock_millis,
    .PortDelay     = mock_delay,
};

static XBee mock_xbee = {
    .htable = &mock_hTable,
    .frameIdCntr = 1
};

// ==== TEST SETUP ====
void setUp(void) {
    mock_xbee.frameIdCntr = 1;
    mock_hTable.PortUartRead = mock_uart_read_valid;
}

void tearDown(void) {}

// ==== TEST CASES ====

void test_asciiToHexArray_valid_input(void) {
    const char *ascii = "1A2B3C4D";
    uint8_t expected[] = {0x1A, 0x2B, 0x3C, 0x4D};
    uint8_t output[4];
    int len = asciiToHexArray(ascii, output, 4);
    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, output, 4);
}

void test_asciiToHexArray_invalid_length(void) {
    const char *ascii = "123";
    uint8_t output[2];
    int len = asciiToHexArray(ascii, output, 2);
    TEST_ASSERT_EQUAL_INT(-1, len);
}

void test_apiSendAtCommand_valid(void) {
    int status = apiSendAtCommand(&mock_xbee, AT_VR, NULL, 0);
    TEST_ASSERT_EQUAL_INT(API_SEND_SUCCESS, status);
}

void test_apiSendAtCommand_invalid(void) {
    int status = apiSendAtCommand(&mock_xbee, (at_command_t)0xFF, NULL, 0);
    TEST_ASSERT_EQUAL_INT(API_SEND_ERROR_INVALID_COMMAND, status);
}

void test_apiSendFrame_valid(void) {
    const uint8_t data[] = {0x01, 0x02, 0x03};
    int status = apiSendFrame(&mock_xbee, 0x10, data, sizeof(data));
    TEST_ASSERT_EQUAL_INT(API_SEND_SUCCESS, status);
}

void test_apiReceiveApiFrame_basic_parse(void) {
    xbee_api_frame_t frame = {0};
    int status = apiReceiveApiFrame(&mock_xbee, &frame);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_SUCCESS, status);
}

void test_apiSendAtCommandAndGetResponse_simulated(void) {
    uint8_t response[16] = {0};
    uint8_t responseLength = 0;
    int status = apiSendAtCommandAndGetResponse(&mock_xbee, AT_VR, NULL, 0,
                                                response, &responseLength, 5000, sizeof(response));
    TEST_ASSERT_EQUAL(API_SEND_SUCCESS, status);
}

void test_xbeeHandleAtResponse_should_print(void) {
    xbee_api_frame_t frame = {
        .data = {0x00, 0x01, 'V', 'R', 0x00, 0x12},
        .length = 6,
        .type = XBEE_API_TYPE_AT_RESPONSE
    };
    xbeeHandleAtResponse(&mock_xbee, &frame);
}

void test_xbeeHandleModemStatus_should_print(void) {
    xbee_api_frame_t frame = {
        .data = {0x00, 0x06},
        .length = 2,
        .type = XBEE_API_TYPE_MODEM_STATUS
    };
    xbeeHandleModemStatus(&mock_xbee, &frame);
}

void test_apiHandleFrame_calls_correct_handler(void) {
    xbee_api_frame_t frame = {
        .type = XBEE_API_TYPE_MODEM_STATUS,
        .data = {0x00, 0x06},
        .length = 2
    };
    apiHandleFrame(&mock_xbee, frame);
}

// ==== Edge Case Tests ====

void test_apiReceiveApiFrame_invalid_start_delimiter(void) {
    mock_hTable.PortUartRead = mock_uart_read_bad_start;
    mock_offset = 0;
    xbee_api_frame_t frame;
    int status = apiReceiveApiFrame(&mock_xbee, &frame);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_ERROR_INVALID_START_DELIMITER, status);
}

void test_apiReceiveApiFrame_incomplete_frame(void) {
    mock_hTable.PortUartRead = mock_uart_read_incomplete;
    mock_offset = 0;
    xbee_api_frame_t frame;
    int status = apiReceiveApiFrame(&mock_xbee, &frame);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_ERROR_TIMEOUT_DATA, status);
}

void test_apiReceiveApiFrame_bad_checksum(void) {
    mock_hTable.PortUartRead = mock_uart_read_bad_checksum;
    mock_offset = 0;
    xbee_api_frame_t frame;
    int status = apiReceiveApiFrame(&mock_xbee, &frame);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_ERROR_INVALID_CHECKSUM, status);
}

void test_apiReceiveApiFrame_null_frame_buffer(void) {
    int status = apiReceiveApiFrame(&mock_xbee, NULL);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_ERROR_NULL_FRAME, status);
}

void test_apiSendAtCommandAndGetResponse_buffer_overflow(void) {
    uint8_t buf[2] = {0};
    uint8_t len = 0;
    int status = apiSendAtCommandAndGetResponse(&mock_xbee, AT_VR, NULL, 0, buf, &len, 5000, 1);
    TEST_ASSERT_EQUAL_INT(API_SEND_ERROR_BUFFER_TOO_SMALL, status);
}