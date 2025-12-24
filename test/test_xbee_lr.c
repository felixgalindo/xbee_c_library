#include "unity.h"
#include "xbee_lr.h"
#include "mock_xbee_api_frames.h"
#include "mock_port.h"
#include "xbee_at_cmds.h"
#include <string.h>
#include <stdlib.h>

// --- GLOBAL TEST OBJECTS AND CALLBACK STUBS ---

static XBee mockXbee;
static XBeeHTable htable;

// Stub callbacks
static void dummyDelay(uint32_t ms) { (void)ms; }
static uint32_t dummyMillis(void) {
    static uint32_t fake_time = 0;
    return fake_time += 500;
}

// --- Callback for Receive Test ---
static bool callbackInvoked = false;
static void onReceive(XBee* self, void* data) {
    (void)self;
    (void)data;
    callbackInvoked = true;
}

static int mockUartInit(uint32_t baud, void* dev) {
    TEST_ASSERT_EQUAL_UINT32(9600, baud);
    TEST_ASSERT_NULL(dev);
    return UART_SUCCESS;
}

// --- SETUP AND TEARDOWN ---

void setUp(void) {
    htable.PortMillis = dummyMillis;
    htable.PortDelay = dummyDelay;
    mockXbee.htable = &htable;
}

void tearDown(void) {}

// --- TEST CASES ---

void test_XBeeLRInit_should_return_true_on_uart_success(void) {
    htable.PortUartInit = mockUartInit;
    TEST_ASSERT_TRUE(XBeeLRInit(&mockXbee, 9600, NULL));
}

void test_XBeeLRConnected_should_return_true_when_response_is_1(void) {
    uint8_t resp[] = {1};
    uint8_t len = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(&mockXbee, AT_AI, NULL, 0, resp, &len, 5000, 33, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRConnected(&mockXbee));
}

void test_XBeeLRConnected_should_return_false_on_error(void) {
    apiSendAtCommandAndGetResponse_ExpectAndReturn(&mockXbee, AT_AI, NULL, 0, NULL, NULL, 5000, 33, -1);
    TEST_ASSERT_FALSE(XBeeLRConnected(&mockXbee));
}

void test_XBeeLRSetAppKey_should_pass_on_valid_input(void) {
    const char *key = "00112233445566778899AABBCCDDEEFF";
    uint8_t response[1] = {0};
    uint8_t len = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(NULL, AT_AK, NULL, 16, response, &len, 5000, 33, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRSetAppKey(NULL, key));
}

void test_XBeeLRSetJoinRX1Delay_should_succeed_with_valid_value(void) {
    uint32_t delay = 5000;
    uint8_t response[2] = {0};
    uint8_t len = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(NULL, AT_J1, NULL, 2, response, &len, 5000, 33, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRSetJoinRX1Delay(NULL, delay));
}

void test_XBeeLRSetRX2Frequency_should_succeed(void) {
    uint32_t freq = 869525000;
    uint8_t response[4] = {0};
    uint8_t len = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(NULL, AT_XF, NULL, 4, response, &len, 5000, 33, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRSetRX2Frequency(NULL, freq));
}

void test_XBeeLRSetAppEUI_should_return_true_for_valid_input(void) {
    const char* appEUI = "A1B2C3D4E5F60708";
    uint8_t response[17] = {0};
    uint8_t respLen = 0;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(&mockXbee, AT_AE, NULL, 8, response, &respLen, 5000, 17, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRSetAppEUI(&mockXbee, appEUI));
}

void test_XBeeLRSetAppEUI_should_return_false_for_invalid_input(void) {
    const char* appEUI = "BADLENGTH";
    TEST_ASSERT_FALSE(XBeeLRSetAppEUI(&mockXbee, appEUI));
}

void test_XBeeLRSetClass_should_send_AT_LC_command(void) {
    uint8_t response[4];
    uint8_t responseLength = 0;
    char classVal = 'A';
    apiSendAtCommandAndGetResponse_ExpectAndReturn(&mockXbee, AT_LC, (const uint8_t*)&classVal, 1, response, &responseLength, 5000, sizeof(response), API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeLRSetClass(&mockXbee, classVal));
}

void test_XBeeLRSendPacket_should_send_and_wait_for_tx_status(void) {
    XBeeLRPacket_t packet = {
        .payload = (uint8_t*)"hi",
        .payloadSize = 2,
        .port = 1,
        .ack = 0
    };
    mockXbee.frameIdCntr = 1;
    mockXbee.txStatusReceived = true;
    mockXbee.deliveryStatus = 0x00;

    apiSendFrame_ExpectAndReturn(&mockXbee, XBEE_API_TYPE_LR_TX_REQUEST, NULL, 5, API_SEND_SUCCESS);
    TEST_ASSERT_EQUAL_UINT8(0x00, XBeeLRSendPacket(&mockXbee, &packet));
}

// void test_XBeeLRHandleTransmitStatus_should_parse_and_set_flags(void) {
//     xbee_api_frame_t frame = {
//         .type = XBEE_API_TYPE_TX_STATUS,
//         .length = 3,
//         .data = {0x00, 0x01, 0x00}
//     };
//     mockXbee.ctable = NULL;
//     XBeeLRHandleTransmitStatus(&mockXbee, &frame);
//     TEST_ASSERT_TRUE(mockXbee.txStatusReceived);
//     TEST_ASSERT_EQUAL_UINT8(0x00, mockXbee.deliveryStatus);
// }

// void test_XBeeLRHandleRxPacket_should_invoke_receive_callback(void) {
//     xbee_api_frame_t frame = {
//         .type = XBEE_API_TYPE_LR_RX_PACKET,
//         .length = 5,
//         .data = {0x81, 0x01, 0x02, 0x03, 0x04}
//     };
//     callbackInvoked = false;
//     XBeeCTable ctable = {.OnReceiveCallback = onReceive};
//     mockXbee.ctable = &ctable;
//     XBeeLRHandleRxPacket(&mockXbee, &frame);
//     TEST_ASSERT_TRUE(callbackInvoked);
// }

// --- UNITY MAIN ---
// int main(void) {
//     UNITY_BEGIN();
//     RUN_TEST(test_XBeeLRInit_should_return_true_on_uart_success);
//     RUN_TEST(test_XBeeLRConnected_should_return_true_when_response_is_1);
//     RUN_TEST(test_XBeeLRConnected_should_return_false_on_error);
//     RUN_TEST(test_XBeeLRSetAppKey_should_pass_on_valid_input);
//     RUN_TEST(test_XBeeLRSetJoinRX1Delay_should_succeed_with_valid_value);
//     RUN_TEST(test_XBeeLRSetRX2Frequency_should_succeed);
//     RUN_TEST(test_XBeeLRSetAppEUI_should_return_true_for_valid_input);
//     RUN_TEST(test_XBeeLRSetAppEUI_should_return_false_for_invalid_input);
//     RUN_TEST(test_XBeeLRSetClass_should_send_AT_LC_command);
//     RUN_TEST(test_XBeeLRSendPacket_should_send_and_wait_for_tx_status);
//     RUN_TEST(test_XBeeLRHandleTransmitStatus_should_parse_and_set_flags);
//     RUN_TEST(test_XBeeLRHandleRxPacket_should_invoke_receive_callback);
//     return UNITY_END();
// }