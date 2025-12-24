/***************************************************************************//**
 * @file xbee_cellular.c
 * @brief Implementation of XBee 3 Cellular subclass.
 *
 * This file contains the implementation of functions specific to the XBee 3 Cellular module.
 * It includes methods for initializing, connecting to LTE networks, sending data,
 * and handling module configuration and runtime events.
 *
 * @version 1.2
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

#include "xbee_cellular.h"
#include "xbee_api_frames.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*****************************************************************************/
/**
 * @brief Initializes the XBee Cellular device with given UART settings.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] baudRate UART baud rate.
 * @param[in] device Pointer to serial device identifier.
 *
 * @return true if UART was initialized successfully, false otherwise.
 ******************************************************************************/
bool XBeeCellularInit(XBee* self, uint32_t baudRate, void* device) {
    return (self->htable->PortUartInit(baudRate, device)) == UART_SUCCESS ? true : false;
}

/*****************************************************************************/
/**
 * @brief Determines if the XBee Cellular module is connected to a network.
 *
 * Sends AT command `AI` to check for network registration.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if connected, false otherwise.
 * @todo Add support for non-blocking connection attempts.
 ******************************************************************************/
bool XBeeCellularConnected(XBee* self) {
    uint8_t response = 0;
    uint8_t responseLength;
    int status = apiSendAtCommandAndGetResponse(self, AT_AI, NULL, 0, &response, &responseLength, 5000, sizeof(response));
    return (status == API_SEND_SUCCESS && response == 0);
}

/*****************************************************************************/
/**
 * @brief Attempts to connect the XBee Cellular module to the LTE network.
 *
 * Applies SIM PIN, APN, and carrier profile settings. Then polls `AI` for attach success.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if registration succeeded, false otherwise.
 ******************************************************************************/
bool XBeeCellularConnect(XBee* self, bool blocking) {
    XBeeCellular* cell = (XBeeCellular*) self;
    XBEEDebugPrint("Applying cellular config and attempting attach...\n");

    if (cell->config.simPin && strlen(cell->config.simPin) > 0) {
        apiSendAtCommand(self, AT_PN, (const uint8_t*)cell->config.simPin, strlen(cell->config.simPin));
    }

    if (cell->config.apn && strlen(cell->config.apn) > 0) {
        XBEEDebugPrint("Setting APN: %s\n", cell->config.apn);
        apiSendAtCommand(self, AT_AN, (const uint8_t*)cell->config.apn, strlen(cell->config.apn));
    }

    if (cell->config.carrier && strlen(cell->config.carrier) > 0) {
        apiSendAtCommand(self, AT_CP, (const uint8_t*)cell->config.carrier, strlen(cell->config.carrier));
    }

    if (!blocking) return true;

    XBEEDebugPrint("Waiting for network attach...\n");
    for (int i = 0; i < 60; ++i) {
        if (XBeeCellularConnected(self)) {
            XBEEDebugPrint("Successfully attached to cellular network.\n");
            return true;
        }
        self->htable->PortDelay(1000);
    }

    XBEEDebugPrint("Network attach failed.\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Gracefully disconnects the XBee Cellular module using AT command.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if AT_SD was accepted successfully.
 ******************************************************************************/
bool XBeeCellularDisconnect(XBee* self) {
    int status = apiSendAtCommand(self, AT_SD, NULL, 0);
    return (status == API_SEND_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief Sends a stateless IPv4 UDP/TCP data packet over the cellular interface.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to a XBeeCellularPacket_t structure.
 *
 * @return 0 if sent successfully, 0xFF on error.
 ******************************************************************************/
uint8_t XBeeCellularSendPacket(XBee* self, const void* data) {
    XBeeCellularPacket_t* packet = (XBeeCellularPacket_t*) data;
    uint8_t frame[128];
    uint8_t frameId = self->frameIdCntr;
    uint8_t offset = 0;

    frame[offset++] = frameId;
    frame[offset++] = packet->protocol;
    frame[offset++] = packet->port >> 8;
    frame[offset++] = packet->port & 0xFF;
    memcpy(&frame[offset], packet->ip, 4); offset += 4;
    memcpy(&frame[offset], packet->payload, packet->payloadSize); offset += packet->payloadSize;

    int status = apiSendFrame(self, XBEE_API_TYPE_CELLULAR_TX_IPV4, frame, offset);
    return (status == API_SEND_SUCCESS) ? 0x00 : 0xFF;
}

/*****************************************************************************/
/**
 * @brief Issues a soft reset via AT command to gracefully shut down the modem.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if shutdown command was sent successfully.
 ******************************************************************************/
bool XBeeCellularSoftReset(XBee* self) {
    return apiSendAtCommand(self, AT_SD, NULL, 0) == API_SEND_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Placeholder for hard reset logic (implementation platform-specific).
 *
 * @param[in] self Pointer to the XBee instance.
 ******************************************************************************/
void XBeeCellularHardReset(XBee* self) {
    (void) self;
}

/*****************************************************************************/
/**
 * @brief Continuously polls for incoming frames from the XBee module.
 *
 * @param[in] self Pointer to the XBee instance.
 ******************************************************************************/
void XBeeCellularProcess(XBee* self) {
    xbee_api_frame_t frame;
    int status = apiReceiveApiFrame(self, &frame);
    if (status == API_SEND_SUCCESS) {
        apiHandleFrame(self, frame);
    }
}

/**
 * @brief Configures the XBee Cellular module with APN, SIM PIN, and carrier settings.
 *
 * This function casts the generic config pointer to an XBeeCellularConfig_t structure and applies
 * the settings to the cellular instance. This is typically called through the base class interface
 * `XBeeConfigure()` which dispatches to this function via the vtable.
 *
 * @param[in] self Pointer to the base XBee instance.
 * @param[in] config Pointer to a valid XBeeCellularConfig_t structure.
 *
 * @return bool Returns true if configuration was applied successfully, otherwise false.
 */
bool XBeeCellularConfigure(XBee* self, const void* config) {
    if (!self || !config) return false;

    XBeeCellular* cell = (XBeeCellular*)self;
    const XBeeCellularConfig_t* cfg = (const XBeeCellularConfig_t*)config;

    // Save config to internal copy
    cell->config = *cfg;
    return true;
}

/*****************************************************************************/
/**
 * @brief Handles incoming Socket Receive (0xCD) and Receive From (0xCE) frames.
 *
 * This function parses both connected socket receive (0xCD) and UDP datagram
 * receive-from (0xCE) frames and dispatches them via the registered callback.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] param Pointer to the received API frame data.
 ******************************************************************************/
static void XBeeCellularHandleRxPacket(XBee* self, void* param) {
    if (!param) {
        XBEEDebugPrint("HandleRxPacket: Null parameter received\n");
        return;
    }

    xbee_api_frame_t* frame = (xbee_api_frame_t*)param;

    if (frame->type != XBEE_API_TYPE_CELLULAR_SOCKET_RX &&
        frame->type != XBEE_API_TYPE_CELLULAR_SOCKET_RX_FROM) {
        XBEEDebugPrint("HandleRxPacket: Unexpected frame type 0x%02X\n", frame->type);
        return;
    }

    XBEEDebugPrint("HandleRxPacket: Frame Length: %u\n", frame->length);

    if ((frame->type == XBEE_API_TYPE_CELLULAR_SOCKET_RX && frame->length < 3) ||
        (frame->type == XBEE_API_TYPE_CELLULAR_SOCKET_RX_FROM && frame->length < 9)) {
        XBEEDebugPrint("HandleRxPacket: Frame too short to be valid\n");
        return;
    }

    const uint8_t* data = frame->data;
    uint8_t frameId = data[0];
    uint8_t socketId = data[1];
    uint8_t status = data[2];

    XBeeCellularPacket_t packet = {0};
    packet.frameId = frameId;
    packet.status = status;
    packet.socketId = socketId;
    packet.protocol = 0xFF; // Not encoded in RX

    if (frame->type == XBEE_API_TYPE_CELLULAR_SOCKET_RX) {
        // Connected socket RX
        packet.payload = (uint8_t*)&data[3];
        packet.payloadSize = frame->length - 3;
        packet.port = 0;
        memset(packet.ip, 0, 4);
    } else {
        // UDP RX_FROM
        memcpy(packet.ip, &data[3], 4);
        packet.remotePort = (data[7] << 8) | data[8];
        packet.payload = (uint8_t*)&data[9];
        packet.payloadSize = frame->length - 9;
        packet.port = packet.remotePort;
    }

    XBEEDebugPrint("HandleRxPacket: FrameID: 0x%02X, Socket ID: %u, Status: 0x%02X\n",
                   frameId, socketId, status);
    XBEEDebugPrint("HandleRxPacket: Payload size: %u bytes\n", packet.payloadSize);

    XBEEDebugPrint("[Payload HEX Dump]:\n");
    for (int i = 0; i < packet.payloadSize && i < 64; ++i) {
        XBEEDebugPrint("%02X ", packet.payload[i]);
    }
    if (packet.payloadSize > 64) XBEEDebugPrint("... [truncated]\n");
    else XBEEDebugPrint("\n");

    XBEEDebugPrint("[Payload ASCII Dump]:\n");
    for (int i = 0; i < packet.payloadSize && i < 64; ++i) {
        char c = packet.payload[i];
        XBEEDebugPrint("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    XBEEDebugPrint("\n");

    if (self->ctable && self->ctable->OnReceiveCallback) {
        self->ctable->OnReceiveCallback(self, &packet);
    }
}

/*****************************************************************************/
/**
 * @brief Virtual function dispatch table for XBeeCellular methods.
 ******************************************************************************/
static const XBeeVTable XBeeCellularVTable = {
    .init = XBeeCellularInit,
    .process = XBeeCellularProcess,
    .connect = XBeeCellularConnect,
    .disconnect = XBeeCellularDisconnect,
    .sendData = XBeeCellularSendPacket,
    .softReset = XBeeCellularSoftReset,
    .hardReset = XBeeCellularHardReset,
    .connected = XBeeCellularConnected,
    .handleRxPacketFrame = XBeeCellularHandleRxPacket,
    .handleTransmitStatusFrame = NULL,
    .configure = XBeeCellularConfigure
};

/*****************************************************************************/
/**
 * @brief Allocates and initializes a new XBeeCellular instance.
 *
 * @param[in] cTable Callback table for RX/TX handlers.
 * @param[in] hTable Platform-specific HAL interface table.
 *
 * @return Pointer to the new XBeeCellular instance.
 ******************************************************************************/
XBeeCellular* XBeeCellularCreate(const XBeeCTable* cTable, const XBeeHTable* hTable) {
    XBeeCellular* instance = (XBeeCellular*)malloc(sizeof(XBeeCellular));
    instance->base.vtable = &XBeeCellularVTable;
    instance->base.ctable = cTable;
    instance->base.htable = hTable;
    return instance;
}

/*****************************************************************************/
/**
 * @brief Frees memory allocated to an XBeeCellular instance.
 *
 * @param[in] self Pointer to the instance to destroy.
 ******************************************************************************/
void XBeeCellularDestroy(XBeeCellular* self) {
    free(self);
}

/*****************************************************************************/
/**
 * @brief Creates a TCP or UDP socket on the XBee Cellular module.
 *
 * Sends a SOCKET_CREATE (0x40) API frame to the XBee module and waits
 * for a corresponding SOCKET_CREATE_RESPONSE (0xC0) frame. On success,
 * it returns the assigned socket ID.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] protocol Socket protocol: 0x01 = TCP, 0x02 = UDP.
 * @param[out] socketIdOut Pointer to a variable to receive the assigned socket ID.
 *
 * @return true if the socket was created successfully, false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketCreate(XBee* self, uint8_t protocol, uint8_t* socketIdOut) {
    if (!self || !socketIdOut) return false;

    uint8_t frameId = self->frameIdCntr++;
    uint8_t frame[2] = { frameId, protocol };

    // Send SOCKET_CREATE request
    if (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_CREATE, frame, 2) != API_SEND_SUCCESS) {
        XBEEDebugPrint("Socket Create: Failed to send frame\n");
        return false;
    }

    // Wait for SOCKET_CREATE_RESPONSE
    xbee_api_frame_t response;
    uint32_t start = self->htable->PortMillis();
    while ((self->htable->PortMillis() - start) < 3000) {
        if (apiReceiveApiFrame(self, &response) == API_SEND_SUCCESS &&
            response.type == XBEE_API_TYPE_CELLULAR_SOCKET_CREATE_RESPONSE) {

            // Check frame ID match and status
            uint8_t respFrameId = response.data[1];
            uint8_t socketId = response.data[2];
            uint8_t status = response.data[3];

            if (respFrameId == frameId && status == 0x00) {
                *socketIdOut = socketId;
                XBEEDebugPrint("Socket Create: Assigned socket ID %u\n", socketId);
                return true;
            } else {
                XBEEDebugPrint("Socket Create: Failed, status 0x%02X\n", status);
                return false;
            }
        }
        self->htable->PortDelay(10);
    }

    XBEEDebugPrint("Socket Create: Timed out waiting for response\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Connects a TCP or UDP socket to a remote IP or hostname using the XBee Cellular module.
 *
 * This function sends a SOCKET_CONNECT (0x42) API frame to the XBee module using either
 * a 4-byte IPv4 address or a null-terminated hostname string, depending on the `isString` flag.
 * It then waits for both a SOCKET_CONNECT_RESPONSE (0xC2) and a SOCKET_STATUS (0xCF) frame
 * to confirm a fully established connection.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId The socket ID previously created with XBeeCellularSocketCreate.
 * @param[in] addr Pointer to the destination address:
 *            - If `isString` is true, this should be a `const char*` hostname (e.g., "example.com").
 *            - If `isString` is false, this should be a `const uint8_t[4]` IP address.
 * @param[in] port The destination port in host byte order.
 * @param[in] isString Set to true to use a hostname string (DNS); false to use IPv4 address.
 *
 * @return true if the connection was successfully established (including socket status), false otherwise.
 * @todo Add support for non-blocking connection attempts.
 ******************************************************************************/
bool XBeeCellularSocketConnect(XBee* self, uint8_t socketId, const void* addr, uint16_t port, bool isString) {
    if (!self || !addr) return false;

    uint8_t frame[128];
    uint8_t offset = 0;
    uint8_t frameId = self->frameIdCntr++;

    frame[offset++] = frameId;
    frame[offset++] = socketId;
    frame[offset++] = port >> 8;
    frame[offset++] = port & 0xFF;

    if (isString) {
        frame[offset++] = 0x01; // Address type: string
        const char* hostname = (const char*)addr;
        size_t len = strlen(hostname);
        if (offset + len > sizeof(frame)) return false;
        memcpy(&frame[offset], hostname, len);
        offset += len;
        XBEEDebugPrint("SocketConnect: Using DNS hostname: %s\n", hostname);
    } else {
        frame[offset++] = 0x00; // Address type: IPv4
        memcpy(&frame[offset], addr, 4);
        offset += 4;
        XBEEDebugPrint("SocketConnect: Using IPv4: %u.%u.%u.%u\n",
            ((uint8_t*)addr)[0], ((uint8_t*)addr)[1], ((uint8_t*)addr)[2], ((uint8_t*)addr)[3]);
    }

    // Send the SOCKET_CONNECT frame
    if (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_CONNECT, frame, offset) != API_SEND_SUCCESS) {
        XBEEDebugPrint("SocketConnect: Failed to send connect frame\n");
        return false;
    }

    // Wait for SOCKET_CONNECT_RESPONSE (0xC2)
    xbee_api_frame_t response;
    uint32_t start = self->htable->PortMillis();
    while ((self->htable->PortMillis() - start) < 3000) {
        if (apiReceiveApiFrame(self, &response) == API_SEND_SUCCESS &&
            response.type == XBEE_API_TYPE_CELLULAR_SOCKET_CONNECT_RESPONSE) {

            if (response.data[1] == frameId &&
                response.data[2] == socketId &&
                response.data[3] == 0x00) {

                XBEEDebugPrint("SocketConnect: Connect response OK\n");
                goto wait_for_status;
            } else {
                XBEEDebugPrint("SocketConnect: Connect failed - status 0x%02X\n", response.data[2]);
                return false;
            }
        }
        self->htable->PortDelay(10);
    }

    XBEEDebugPrint("SocketConnect: Timed out waiting for connect response\n");
    return false;

wait_for_status:
    // Wait for SOCKET_STATUS (0xCF) to confirm socket is connected
    start = self->htable->PortMillis();
    while ((self->htable->PortMillis() - start) < 20000) {
        if (apiReceiveApiFrame(self, &response) == API_SEND_SUCCESS &&
            response.type == XBEE_API_TYPE_CELLULAR_SOCKET_STATUS) {

            if (response.data[1] == socketId && response.data[2] == 0x00) {
                XBEEDebugPrint("SocketConnect: Socket status CONNECTED\n");
                return true;
            } else {
                XBEEDebugPrint("SocketConnect: Unexpected socket status 0x%02X\n", response.data[2]);
                return false;
            }
        }
        self->htable->PortDelay(10);
    }

    XBEEDebugPrint("SocketConnect: Timed out waiting for socket status\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Sends data over a previously connected socket.
 *
 * Constructs a SOCKET_SEND API frame to transmit the payload.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId Socket ID to send through.
 * @param[in] payload Pointer to data buffer.
 * @param[in] payloadLen Length of data to send.
 *
 * @return true if send was accepted, false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketSend(XBee* self, uint8_t socketId, const uint8_t* payload, uint16_t payloadLen) {
    if (!payload || payloadLen == 0 || payloadLen > 120) return false;

    uint8_t frame[128];
    uint8_t offset = 0;
    frame[offset++] = self->frameIdCntr++;
    frame[offset++] = socketId;
    frame[offset++] = 0x00; //Transmit options
    memcpy(&frame[offset], payload, payloadLen);
    offset += payloadLen;

    return (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_SEND, frame, offset) == API_SEND_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief Sets an option on the socket (e.g., bind, listen).
 *
 * Sends a SOCKET_OPTION API frame to the XBee module.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId Socket ID to configure.
 * @param[in] option Option number to set.
 * @param[in] value Pointer to value data.
 * @param[in] valueLen Number of bytes in value.
 *
 * @return true if option was set successfully, false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketSetOption(XBee* self, uint8_t socketId, uint8_t option, const uint8_t* value, uint8_t valueLen) {
    if (!value || valueLen == 0) return false;

    uint8_t frame[128];
    uint8_t offset = 0;
    frame[offset++] = self->frameIdCntr++;
    frame[offset++] = socketId;
    frame[offset++] = option;
    memcpy(&frame[offset], value, valueLen);
    offset += valueLen;

    return (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_OPTION, frame, offset) == API_SEND_SUCCESS);
}


/*****************************************************************************/
/**
 * @brief Closes an open socket on the XBee Cellular module.
 *
 * This function sends a SOCKET_CLOSE (0x43) API frame to the XBee module with the specified
 * socket ID. If `blocking` is true, it waits for a SOCKET_STATUS (0xCF) frame indicating
 * that the socket was successfully closed.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId The socket ID to close.
 * @param[in] blocking If true, waits for close confirmation via 0xCF frame.
 *
 * @return true if close frame sent (and confirmed if blocking), false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketClose(XBee* self, uint8_t socketId, bool blocking) {
    if (!self) return false;

    uint8_t frameId = self->frameIdCntr++;
    uint8_t frame[2] = { frameId, socketId };

    if (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_CLOSE, frame, sizeof(frame)) != API_SEND_SUCCESS) {
        XBEEDebugPrint("SocketClose: Failed to send close frame\n");
        return false;
    }

    if (!blocking)
        return true;

    // Wait for SOCKET_STATUS (0xCF) response indicating closure
    xbee_api_frame_t response;
    uint32_t start = self->htable->PortMillis();
    while ((self->htable->PortMillis() - start) < 3000) {
        if (apiReceiveApiFrame(self, &response) == API_SEND_SUCCESS &&
            response.type == XBEE_API_TYPE_CELLULAR_SOCKET_STATUS) {

            if (response.data[1] == frameId &&
                response.data[2] == socketId &&
                response.data[3] == 0x01) {  // 0x01 = closed
                XBEEDebugPrint("SocketClose: Socket %u closed\n", socketId);
                return true;
            }

            XBEEDebugPrint("SocketClose: Unexpected socket status (ID: %u, Status: 0x%02X)\n",
                           response.data[2], response.data[3]);
            return false;
        }
        self->htable->PortDelay(10);
    }

    XBEEDebugPrint("SocketClose: Timeout waiting for socket status\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Binds a UDP socket to a local port on the XBee Cellular module.
 *
 * This function sends a SOCKET_BIND (0x46) API frame to the XBee module,
 * specifying the socket ID and local port number. If blocking is enabled,
 * the function waits for a corresponding SOCKET_BIND_RESPONSE (0xC6) frame
 * indicating the bind result.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId The socket ID to bind.
 * @param[in] port Local port to bind to (host byte order).
 * @param[in] blocking If true, waits for a bind response from the module.
 *
 * @return true if bind frame was sent (and acknowledged if blocking), false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketBind(XBee* self, uint8_t socketId, uint16_t port, bool blocking) {
    if (!self) return false;

    uint8_t frameId = self->frameIdCntr++;
    uint8_t frame[4];
    uint8_t offset = 0;
    frame[offset++] = frameId;
    frame[offset++] = socketId;
    frame[offset++] = (port >> 8) & 0xFF;
    frame[offset++] = port & 0xFF;

    if (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_BIND, frame, offset) != API_SEND_SUCCESS) {
        XBEEDebugPrint("SocketBind: Failed to send bind frame\n");
        return false;
    }

    if (!blocking)
        return true;

    // Wait for SOCKET_BIND_RESPONSE (0xC6)
    xbee_api_frame_t response;
    uint32_t start = self->htable->PortMillis();
    while ((self->htable->PortMillis() - start) < 3000) {
        if (apiReceiveApiFrame(self, &response) == API_SEND_SUCCESS &&
            response.type == XBEE_API_TYPE_CELLULAR_SOCKET_BIND_RESPONSE) {

            if (response.data[1] == frameId &&
                response.data[2] == socketId &&
                response.data[3] == 0x00) {  // 0x00 = success
                XBEEDebugPrint("SocketBind: Socket %u bound to port 0x%04X\n", socketId, port);
                return true;
            }

            XBEEDebugPrint("SocketBind: Unexpected bind status (ID: %u, Status: 0x%02X)\n",
                           response.data[2], response.data[3]);
            return false;
        }
        self->htable->PortDelay(10);
    }

    XBEEDebugPrint("SocketBind: Timeout waiting for bind status\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Sends a UDP packet to a specific IP:port using an open socket.
 *
 * Constructs a SOCKET_SEND_TO (0x45) API frame and transmits it.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] socketId Socket ID to use for sending.
 * @param[in] ip Destination IPv4 address (4 bytes).
 * @param[in] port Destination port (host byte order).
 * @param[in] payload Pointer to data buffer.
 * @param[in] payloadLen Length of data to send (1–120 bytes).
 *
 * @return true if the frame was sent successfully, false otherwise.
 ******************************************************************************/
bool XBeeCellularSocketSendTo(XBee* self, uint8_t socketId, const uint8_t* ip, uint16_t port,
                              const uint8_t* payload, uint16_t payloadLen) {
    if (!self || !ip || !payload || payloadLen == 0 || payloadLen > 120) return false;

    uint8_t frame[128];
    uint8_t offset = 0;

    frame[offset++] = self->frameIdCntr++;
    frame[offset++] = socketId;
    memcpy(&frame[offset], ip, 4); offset += 4;
    frame[offset++] = (port >> 8) & 0xFF;
    frame[offset++] = port & 0xFF;
    frame[offset++] = 0x00;  // Transmit options
    memcpy(&frame[offset], payload, payloadLen); offset += payloadLen;

    return (apiSendFrame(self, XBEE_API_TYPE_CELLULAR_SOCKET_SEND_TO, frame, offset) == API_SEND_SUCCESS);
}
