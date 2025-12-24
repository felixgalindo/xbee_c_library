#include "unity.h"
#include "xbee_cellular.h"
#include "mock_xbee_api_frames.h"
#include "mock_port.h"
#include "xbee_at_cmds.h"
#include <string.h>

// --- GLOBALS ---

static XBeeCellular mockCellular;
static XBee* self = (XBee*)&mockCellular;
static XBeeHTable htable;
static XBeeCTable ctable;
static uint32_t fake_time;

// --- STUBS ---

static int dummyUartInit(uint32_t baud, void* dev) {
    TEST_ASSERT_EQUAL_UINT32(9600, baud);
    TEST_ASSERT_EQUAL_STRING("COM1", (char*)dev);
    return UART_SUCCESS;
}

static void dummyDelay(uint32_t ms) {
    (void)ms;
}

static uint32_t dummyMillis(void) {
    return fake_time += 500;
}

// --- SETUP ---

void setUp(void) {
    fake_time = 0;
    memset(&mockCellular, 0, sizeof(mockCellular));

    htable.PortUartInit = dummyUartInit;
    htable.PortDelay = dummyDelay;
    htable.PortMillis = dummyMillis;

    mockCellular.base.htable = &htable;
    mockCellular.base.ctable = &ctable;
}

void tearDown(void) {}

// --- TESTS ---

void test_XBeeCellularInit_should_return_true_when_uart_init_succeeds(void) {
    TEST_ASSERT_TRUE(XBeeCellularInit(self, 9600, "COM1"));
}

void test_XBeeCellularConnected_should_return_true_when_AI_is_zero(void) {
    uint8_t dummyResponse = 0;
    uint8_t dummyResponseLength = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(
        self, AT_AI, NULL, 0, &dummyResponse, &dummyResponseLength, 5000, sizeof(dummyResponse), API_SEND_SUCCESS
    );
    TEST_ASSERT_TRUE(XBeeCellularConnected(self));
}

void test_XBeeCellularConnect_should_return_false_when_attach_fails(void) {
    mockCellular.config.simPin = "";
    mockCellular.config.apn = "";
    mockCellular.config.carrier = "";
    TEST_ASSERT_FALSE(XBeeCellularConnect(self, true));
}

void test_XBeeCellularDisconnect_should_send_AT_SD_and_return_true(void) {
    apiSendAtCommand_ExpectAndReturn(self, AT_SD, NULL, 0, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularDisconnect(self));
}

void test_XBeeCellularSendPacket_should_return_success_when_frame_sent(void) {
    XBeeCellularPacket_t pkt = {
        .protocol = 1,
        .port = 80,
        .payload = (uint8_t*)"test",
        .payloadSize = 4
    };
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_TX_IPV4, NULL, 11, API_SEND_SUCCESS);
    TEST_ASSERT_EQUAL_HEX8(0x00, XBeeCellularSendPacket(self, &pkt));
}

void test_XBeeCellularSoftReset_should_send_AT_SD(void) {
    apiSendAtCommand_ExpectAndReturn(self, AT_SD, NULL, 0, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularSoftReset(self));
}

void test_XBeeCellularHardReset_should_not_crash(void) {
    XBeeCellularHardReset(self);
}

void test_XBeeCellularConfigure_should_copy_config(void) {
    XBeeCellularConfig_t config = {
        .simPin = "1234",
        .apn = "internet",
        .carrier = "verizon"
    };
    TEST_ASSERT_TRUE(XBeeCellularConfigure(self, &config));
    TEST_ASSERT_EQUAL_STRING("internet", mockCellular.config.apn);
    TEST_ASSERT_EQUAL_STRING("verizon", mockCellular.config.carrier);
    TEST_ASSERT_EQUAL_STRING("1234", mockCellular.config.simPin);
}

void test_XBeeCellularSocketCreate_should_return_socket_id_on_success(void) {
    uint8_t socketId;
    xbee_api_frame_t response = {
        .type = XBEE_API_TYPE_CELLULAR_SOCKET_CREATE_RESPONSE,
        .data = {0, 1, 0x12, 0x00}
    };

    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_CREATE, NULL, 2, API_SEND_SUCCESS);
    apiReceiveApiFrame_ExpectAndReturn(self, NULL, API_SEND_SUCCESS);

    TEST_ASSERT_TRUE(XBeeCellularSocketCreate(self, 0x01, &socketId));
}

void test_XBeeCellularSocketSend_should_return_false_on_null_payload(void) {
    TEST_ASSERT_FALSE(XBeeCellularSocketSend(self, 1, NULL, 0));
}

void test_XBeeCellularSocketSetOption_should_send_option(void) {
    uint8_t value[2] = {0x01, 0x02};
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_OPTION, NULL, 5, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularSocketSetOption(self, 1, 2, value, sizeof(value)));
}

void test_XBeeCellularSocketClose_should_return_false_on_send_failure(void) {
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_CLOSE, NULL, 2, API_SEND_ERROR_UART_FAILURE);
    TEST_ASSERT_FALSE(XBeeCellularSocketClose(self, 2));
}

// --- MAIN (optional) ---
// int main(void) {
//     UNITY_BEGIN();
//     RUN_TEST(test_XBeeCellularInit_should_return_true_when_uart_init_succeeds);
//     RUN_TEST(test_XBeeCellularConnected_should_return_true_when_AI_is_zero);
//     RUN_TEST(test_XBeeCellularConnect_should_return_false_when_attach_fails);
//     RUN_TEST(test_XBeeCellularDisconnect_should_send_AT_SD_and_return_true);
//     RUN_TEST(test_XBeeCellularSendPacket_should_return_success_when_frame_sent);
//     RUN_TEST(test_XBeeCellularSoftReset_should_send_AT_SD);
//     RUN_TEST(test_XBeeCellularHardReset_should_not_crash);
//     RUN_TEST(test_XBeeCellularConfigure_should_copy_config);
//     RUN_TEST(test_XBeeCellularSocketCreate_should_return_socket_id_on_success);
//     RUN_TEST(test_XBeeCellularSocketSend_should_return_false_on_null_payload);
//     RUN_TEST(test_XBeeCellularSocketSetOption_should_send_option);
//     RUN_TEST(test_XBeeCellularSocketClose_should_return_false_on_send_failure);
//     return UNITY_END();
// }