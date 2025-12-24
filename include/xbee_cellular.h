/***************************************************************************//**
 * @file xbee_cellular.h
 * @brief Header for XBee 3 Cellular API.
 *
 * Declares the registration and high-level management functions for
 * XBee 3 Cellular LTE/NB-IoT modems.
 *
 * @version 1.1
 * @date 2025-05-14
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
 ******************************************************************************/

#ifndef XBEE_CELLULAR_H
#define XBEE_CELLULAR_H

#include "xbee.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Supported socket protocols.
 */
typedef enum {
    XBEE_PROTOCOL_UDP = 0x00,  
    XBEE_PROTOCOL_TCP = 0x01,
    XBEE_PROTOCOL_SSL = 0x04
} xbee_protocol_t;

/**
 * @brief Supported socket option identifiers.
 */
typedef enum {
    XBEE_SOCKET_OPT_BIND_PORT = 0x00,
    XBEE_SOCKET_OPT_LISTEN    = 0x01,
    XBEE_SOCKET_OPT_KEEPALIVE = 0x02
} xbee_socket_option_t;

/***************************************************************************//**
 * @struct XBeeCellularPacket_t
 * @brief Structure representing a packet for XBee 3 Cellular modules.
 *
 * This structure is used to encapsulate both TX and RX packet fields for
 * TCP/UDP communication via IPv4 over cellular networks. The structure
 * supports bidirectional packet operations and is compatible with API frames
 * 0x20 (TX) and 0xB0 (RX).
 ******************************************************************************/
typedef struct XBeeCellularPacket_s {
    // Common
    uint8_t protocol;          ///< 0x01 = TCP, 0x02 = UDP, 0x04 = SSL
    uint16_t port;             ///< Destination port (TX) or local port (RX)
    uint8_t ip[4];             ///< IPv4 address (destination or source)
    uint8_t* payload;          ///< Pointer to payload data
    uint16_t payloadSize;       ///< Length of payload in bytes

    // TX-specific
    uint8_t frameId;           ///< Frame ID used for tracking TX responses

    // RX-specific
    uint8_t socketId;          ///< Socket ID on which data was received
    uint16_t remotePort;       ///< Source port of incoming packet
    uint8_t status;            ///< Reserved status byte in RX frames
} XBeeCellularPacket_t;

/**
 * @brief User configuration parameters for cellular operation.
 */
typedef struct {
    const char* apn;        ///< APN string (e.g. "hologram")
    const char* simPin;     ///< SIM unlock PIN (optional)
    const char* carrier;    ///< Carrier profile (optional)
} XBeeCellularConfig_t;

/**
 * @brief XBeeCellular instance derived from base XBee class.
 */
typedef struct {
    XBee base;
    XBeeCellularConfig_t config;
} XBeeCellular;

/**
 * @brief Applies the given APN/SIM/carrier configuration to a cellular instance.
 */
bool XBeeCellularConfigure(XBee* self, const void* config);

/**
 * @brief Initializes the UART interface for the module.
 */
bool XBeeCellularInit(XBee* self, uint32_t baudRate, void* device);

/**
 * @brief Starts LTE network registration and waits until attached.
 */
bool XBeeCellularConnect(XBee* self, bool blocking);

/**
 * @brief Gracefully disconnects from the cellular network.
 */
bool XBeeCellularDisconnect(XBee* self);

/**
 * @brief Returns true if the module is currently connected.
 */
bool XBeeCellularConnected(XBee* self);

/**
 * @brief Sends a payload using UDP or TCP over cellular.
 */
uint8_t XBeeCellularSendPacket(XBee* self, const void* packet);

/**
 * @brief Issues a soft reset using AT commands.
 */
bool XBeeCellularSoftReset(XBee* self);

/**
 * @brief Invokes platform-specific hard reset logic.
 */
void XBeeCellularHardReset(XBee* self);

/**
 * @brief Processes incoming frames and dispatches handlers.
 */
void XBeeCellularProcess(XBee* self);

/**
 * @brief Allocates and initializes an XBeeCellular instance.
 */
XBeeCellular* XBeeCellularCreate(const XBeeCTable* cTable, const XBeeHTable* hTable);

/**
 * @brief Deallocates the XBeeCellular instance.
 */
void XBeeCellularDestroy(XBeeCellular* self);

/**
 * @brief Creates a new socket on the XBee module.
 */
bool XBeeCellularSocketCreate(XBee* self, uint8_t protocol, uint8_t* socketIdOut);

/**
 * @brief Connects an existing socket to a remote address.
 */
bool XBeeCellularSocketConnect(XBee* self, uint8_t socketId, const void* addr, uint16_t port, bool isString);

/**
 * @brief Sends raw data over a specified socket.
 */
bool XBeeCellularSocketSend(XBee* self, uint8_t socketId, const uint8_t* payload, uint16_t payloadLen);

/**
 * @brief Sets a socket option (e.g., bind port or listen mode).
 */
bool XBeeCellularSocketSetOption(XBee* self, uint8_t socketId, uint8_t option, const uint8_t* value, uint8_t valueLen);

/**
 * @brief Closes the specified socket.
 */
bool XBeeCellularSocketClose(XBee* self, uint8_t socketId, bool blocking);

/**
 * @brief Binds a socket to a local UDP port.
 */
bool XBeeCellularSocketBind(XBee* self, uint8_t socketId, uint16_t port, bool blocking);

/**
 * @brief Sends a UDP datagram to a remote IP and port.
 */
bool XBeeCellularSocketSendTo(XBee* self, uint8_t socketId, const uint8_t* ip, uint16_t port,
                              const uint8_t* payload, uint16_t payloadLen);

#ifdef __cplusplus
}
#endif

#endif // XBEE_CELLULAR_H
