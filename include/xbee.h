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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
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
    bool (*init)(XBee* self, uint32_t baudrate, const char* device);
    bool (*connect)(XBee* self);
    bool (*disconnect)(XBee* self);
    bool (*send_data)(XBee* self, const void* data);
    bool (*soft_reset)(XBee* self);
    void (*hard_reset)(XBee* self);
    void (*process)(XBee* self);
    bool (*connected)(XBee* self);
    void (*handle_rx_packet_frame)(XBee* self, void *frame);
    void (*handle_transmit_status_frame)(XBee* self, void *frame);
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
    uart_status_t (*PortUartRead)(uint8_t *buf, int len, int *bytes_read);
    int (*PortUartWrite)(uint8_t *data, uint16_t len);
    uint32_t (*PortMillis)(void);
    void (*PortFlushRx)(void);
    int (*PortUartInit)(uint32_t baudrate, const char *device);
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
};

// Interface functions to call the methods
bool XBee_Init(XBee* self, uint32_t baudrate, const char* device);
bool XBee_Connect(XBee* self);
bool XBee_Disconnect(XBee* self);
bool XBee_SendData(XBee* self, const void*);
bool XBee_SoftReset(XBee* self);
void XBee_HardReset(XBee* self);
void XBee_Process(XBee* self);
bool XBee_Connected(XBee* self);
bool XBee_WriteConfig(XBee* self);
bool XBee_ApplyChanges(XBee* self);

#endif // XBEE_H
