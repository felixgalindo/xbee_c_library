/**
 * @file xbee.h
 * @brief Header file for the XBee class.
 * 
 * This file defines the interface for the XBee class, including
 * the necessary functions, data structures, and constants required to interact
 * with XBee modules using the API frame format. It provides prototypes for
 * initializing the module, sending and receiving data, and handling AT commands.
 * 
 * @version 1.0
 * @date 2024-08-08
 * 
 * @license MIT
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * @author Felix Galindo
 * @contact felix.galindo@digi.com
 */

#ifndef XBEE_H
#define XBEE_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "config.h"
#include "port.h"

// Abstract base class for XBee
typedef struct XBee XBee;

/**
 * @typedef XBeeVTable
 * @brief Virtual table structure for platform-specific XBee operations.
 * 
 * This structure contains function pointers to XBee subclass specific implementations
 * of various XBee operations, such as initialization, sending data, and receiving data.
 * The use of a virtual table allows the XBee library to abstract XBee differences,
 * enabling the same code to run on different hardware or operating systems by providing
 * the appropriate function implementations for each platform.
 */
typedef struct {
    bool (*init)(XBee* self, uint32_t baudrate, void* device);
    bool (*connect)(XBee* self, bool blocking);
    bool (*disconnect)(XBee* self);
    uint8_t (*sendData)(XBee* self, const void* data);
    bool (*softReset)(XBee* self);
    void (*hardReset)(XBee* self);
    void (*process)(XBee* self);
    bool (*connected)(XBee* self);
    void (*handleRxPacketFrame)(XBee* self, void *frame);
    void (*handleTransmitStatusFrame)(XBee* self, void *frame);
    bool (*configure)(XBee* self, const void* config);
} XBeeVTable;


/**
 * @typedef XBeeHTable
 * @brief Virtual table structure for platform-specific XBee operations.
 * 
 * This structure contains function pointers to hardware specific implementations
 * of various XBee operations, such as uart reads & writes, delays, etc.
 * The use of a virtual table allows the XBee library to abstract platform differences,
 * enabling the same code to run on different hardware or operating systems by providing
 * the appropriate function implementations for each platform.
 */
typedef struct {
    int (*PortUartRead)(uint8_t *buffer, int length);
    int (*PortUartWrite)(const uint8_t *buf, uint16_t len);
    uint32_t (*PortMillis)(void);
    void (*PortFlushRx)(void);
    int (*PortUartInit)(uint32_t baudrate, void *device);
    void (*PortDelay)(uint32_t ms);
} XBeeHTable;

/**
 * @typedef XBeeCTable
 * @brief Callback table structure for XBee event handling.
 * 
 * This structure contains function pointers to callback functions that handle 
 * various events related to the XBee module. These callbacks are invoked when 
 * specific events occur, such as receiving data, connecting, disconnecting, 
 * or sending data. The callback table allows the application to customize 
 * the behavior of the XBee module by assigning appropriate functions to each event.
 */
typedef struct {
    void (*OnReceiveCallback)(XBee* self, void * data);
    void (*OnConnectCallback)(XBee* self);
    void (*OnDisconnectCallback)(XBee* self);
    void (*OnSendCallback)(XBee* self, void * data);
} XBeeCTable;

/**
 * @typedef XBee
 * @brief Represents an XBee device instance.
 * 
 * This structure contains the necessary methods for interacting
 * with an XBee module, including connecting, sending data, and receiving data.
 * Contains function pointers for XBee subclass specific operations.
 */
struct XBee {
    // Add common XBee attributes here
    const XBeeVTable* vtable;
    const XBeeHTable* htable;
    const XBeeCTable* ctable;
    uint8_t frameIdCntr;
    bool txStatusReceived;        ///< Flag to indicate if TX Status frame was received
    uint8_t deliveryStatus;        ///< Stores the delivery status of the transmitted frame

};

// Interface functions to call the methods
bool XBeeInit(XBee* self, uint32_t baudrate, void* device);
bool XBeeConnect(XBee* self, bool blocking);
bool XBeeDisconnect(XBee* self);
uint8_t XBeeSendPacket(XBee* self, const void*);
bool XBeeSoftReset(XBee* self);
void XBeeHardReset(XBee* self);
void XBeeProcess(XBee* self);
bool XBeeConnected(XBee* self);
bool XBeeWriteConfig(XBee* self);
bool XBeeApplyChanges(XBee* self);
bool XBeeSetAPIOptions(XBee* self, const uint8_t value);
bool XBeeGetFirmwareVersion(XBee* self, uint32_t* version);
bool XBeeConfigure(XBee* self, const void* config);
bool XBeeFactoryReset        (XBee* self);                          /* ATFR */
bool XBeeExitCommandMode     (XBee* self);                          /* ATCN */
bool XBeeSetApiEnable        (XBee* self, uint8_t mode);            /* ATAP */
bool XBeeSetBaudRate         (XBee* self, uint8_t rateCode);        /* ATBD */
bool XBeeGetLastRssi         (XBee* self, int8_t*  rssiOut);        /* ATDB */
bool XBeeGetHardwareVersion  (XBee* self, uint16_t* hvOut);         /* ATHV */
bool XBeeGetSerialNumber     (XBee* self, uint64_t* snOut);         /* ATSH/ATSL */

#if defined(__cplusplus)
}
#endif

#endif // XBEE_H
