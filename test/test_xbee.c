#include "unity.h"
#include "xbee.h"
#include "xbee.h"
#include "xbee_api_frames.h" 
#include "xbee_at_cmds.h"
#include <stdint.h>
#include <stdbool.h>

// ----------------------------
// Mock Flags for Verification
// ----------------------------
static bool mockInitCalled = false;
static bool mockConnectCalled = false;
static bool mockDisconnectCalled = false;
static bool mockSendDataCalled = false;
static bool mockSoftResetCalled = false;
static bool mockHardResetCalled = false;
static bool mockProcessCalled = false;
static bool mockConnectedCalled = false;
static bool mockConfigureCalled = false;

// ----------------------------
// Mock Implementations
// ---------------------------- 

uint32_t portMillis(void) {
    static uint32_t fakeTime = 0;
    return fakeTime += 100;
}

void portDelay(uint32_t ms) {
    (void)ms;
}

static bool MockInit(XBee* self, uint32_t baudRate, void* device) {
    mockInitCalled = true;
    return baudRate == 9600 && device == NULL;
}

static bool MockConnect(XBee* self, bool blocking) {
    mockConnectCalled = true;
    return blocking;
}

static bool MockDisconnect(XBee* self) {
    mockDisconnectCalled = true;
    return true;
}

static uint8_t MockSendData(XBee* self, const void* data) {
    mockSendDataCalled = true;
    return 0x00;  // Assume success
}

static bool MockSoftReset(XBee* self) {
    mockSoftResetCalled = true;
    return true;
}

static void MockHardReset(XBee* self) {
    mockHardResetCalled = true;
}

static void MockProcess(XBee* self) {
    mockProcessCalled = true;
}

static bool MockConnected(XBee* self) {
    mockConnectedCalled = true;
    return true;
}

static bool MockConfigure(XBee* self, const void* config) {
    mockConfigureCalled = true;
    return true;
}

// ----------------------------
// Test Setup
// ----------------------------
static XBeeVTable mockVTable = {
    .init = MockInit,
    .connect = MockConnect,
    .disconnect = MockDisconnect,
    .sendData = MockSendData,
    .softReset = MockSoftReset,
    .hardReset = MockHardReset,
    .process = MockProcess,
    .connected = MockConnected,
    .configure = MockConfigure
};

static XBee xbee;

void setUp(void) {
    xbee.vtable = &mockVTable;
    xbee.frameIdCntr = 0;

    mockInitCalled = false;
    mockConnectCalled = false;
    mockDisconnectCalled = false;
    mockSendDataCalled = false;
    mockSoftResetCalled = false;
    mockHardResetCalled = false;
    mockProcessCalled = false;
    mockConnectedCalled = false;
    mockConfigureCalled = false;
}

void tearDown(void) {}

// ----------------------------
// Unit Tests
// ----------------------------

void test_XBeeInit_ShouldCallInitAndSetFrameId(void) {
    TEST_ASSERT_TRUE(XBeeInit(&xbee, 9600, NULL));
    TEST_ASSERT_EQUAL(1, xbee.frameIdCntr);
    TEST_ASSERT_TRUE(mockInitCalled);
}

void test_XBeeConnect_ShouldCallConnectWithBlockingTrue(void) {
    TEST_ASSERT_TRUE(XBeeConnect(&xbee, true));
    TEST_ASSERT_TRUE(mockConnectCalled);
}

void test_XBeeConnect_ShouldCallConnectWithBlockingFalse(void) {
    TEST_ASSERT_FALSE(XBeeConnect(&xbee, false));
    TEST_ASSERT_TRUE(mockConnectCalled);
}

void test_XBeeDisconnect_ShouldReturnTrueAndCallDisconnect(void) {
    TEST_ASSERT_TRUE(XBeeDisconnect(&xbee));
    TEST_ASSERT_TRUE(mockDisconnectCalled);
}

void test_XBeeSendData_ShouldReturnSuccessStatus(void) {
    uint8_t dummyPayload = 0xAB;
    TEST_ASSERT_EQUAL_UINT8(0x00, XBeeSendData(&xbee, &dummyPayload));
    TEST_ASSERT_TRUE(mockSendDataCalled);
}

void test_XBeeSoftReset_ShouldReturnTrue(void) {
    TEST_ASSERT_TRUE(XBeeSoftReset(&xbee));
    TEST_ASSERT_TRUE(mockSoftResetCalled);
}

void test_XBeeHardReset_ShouldCallMockHardReset(void) {
    XBeeHardReset(&xbee);
    TEST_ASSERT_TRUE(mockHardResetCalled);
}

void test_XBeeProcess_ShouldCallProcess(void) {
    XBeeProcess(&xbee);
    TEST_ASSERT_TRUE(mockProcessCalled);
}

void test_XBeeConnected_ShouldReturnTrue(void) {
    TEST_ASSERT_TRUE(XBeeConnected(&xbee));
    TEST_ASSERT_TRUE(mockConnectedCalled);
}

void test_XBeeConfigure_ShouldReturnTrue_WhenSupported(void) {
    TEST_ASSERT_TRUE(XBeeConfigure(&xbee, NULL));
    TEST_ASSERT_TRUE(mockConfigureCalled);
}

void test_XBeeConfigure_ShouldReturnFalse_WhenNotSupported(void) {
    // Copy vtable to a mutable version
    XBeeVTable mutableVTable = *xbee.vtable;
    mutableVTable.configure = NULL;
    xbee.vtable = &mutableVTable;

    TEST_ASSERT_FALSE(XBeeConfigure(&xbee, NULL));
}

// ----------------------------
// Main Runner (optional)
// ----------------------------
// int main(void) {
//     UNITY_BEGIN();
//     RUN_TEST(test_XBeeInit_ShouldCallInitAndSetFrameId);
//     RUN_TEST(test_XBeeConnect_ShouldCallConnectWithBlockingTrue);
//     RUN_TEST(test_XBeeConnect_ShouldCallConnectWithBlockingFalse);
//     RUN_TEST(test_XBeeDisconnect_ShouldReturnTrueAndCallDisconnect);
//     RUN_TEST(test_XBeeSendData_ShouldReturnSuccessStatus);
//     RUN_TEST(test_XBeeSoftReset_ShouldReturnTrue);
//     RUN_TEST(test_XBeeHardReset_ShouldCallMockHardReset);
//     RUN_TEST(test_XBeeProcess_ShouldCallProcess);
//     RUN_TEST(test_XBeeConnected_ShouldReturnTrue);
//     RUN_TEST(test_XBeeConfigure_ShouldReturnTrue_WhenSupported);
//     RUN_TEST(test_XBeeConfigure_ShouldReturnFalse_WhenNotSupported);
//     return UNITY_END();
// }