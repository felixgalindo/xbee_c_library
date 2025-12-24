/***************************************************************************//**
 * @file xbee_cellular_http_get_example.c
 * @brief Example application demonstrating the use of the XBeeCellular driver.
 *
 * This file contains a sample application that demonstrates how to use the
 * XBeeCellular library to communicate with XBee 3 Cellular modules using a
 * portable, platform-agnostic interface. It showcases basic operations such as
 * initializing the module, configuring network settings, attaching to the
 * cellular network, creating a TCP socket, sending an HTTP GET request, and
 * printing the response.
 *
 * @version 1.2
 * @date 2025-05-20
 * @author Felix Galindo
 * @contact felix.galindo@digi.com
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
 ******************************************************************************/

#include "xbee_cellular.h"
#include "port.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


// Determine default serial device path based on platform (replace with your actual device path)
// This is a placeholder; you may need to adjust the path based on your system.
#if defined(_WIN32)
    #define DEFAULT_SERIAL_PORT "COM3"
#elif defined(__APPLE__)
    #define DEFAULT_SERIAL_PORT "/dev/cu.usbserial-1110"
#elif defined(__linux__)
    #define DEFAULT_SERIAL_PORT "/dev/ttyUSB0"
#else
    #define DEFAULT_SERIAL_PORT ""
#endif

static bool responseReceived = false;

/**
 * @brief Callback triggered when data is received via SOCKET_RECEIVE (0xCD)
 *        or SOCKET_RECEIVE_FROM (0xCE) frame.
 *
 * This function prints the payload from the remote peer in plain text format.
 * If the source IP and port are known (UDP), they are printed as well.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to the XBeeCellularPacket_t payload descriptor.
 */
void OnReceiveCallback(XBee* self, void* data) {
    (void)self;
    XBeeCellularPacket_t* packet = (XBeeCellularPacket_t*)data;

    responseReceived = true;

    if (packet->ip[0] != 0 || packet->remotePort != 0) {
        portDebugPrintf("Received from %u.%u.%u.%u:%u on socket %u\n",
            packet->ip[0], packet->ip[1], packet->ip[2], packet->ip[3],
            packet->remotePort, packet->socketId);
    } else {
        portDebugPrintf("Received on socket %u\n", packet->socketId);
    }

    portDebugPrintf("[Payload ASCII Dump]:\n");
    for (int i = 0; i < packet->payloadSize; ++i) {
        char c = packet->payload[i];
        portDebugPrintf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    portDebugPrintf("\n");
}

/**
 * Callback triggered after a frame has been transmitted successfully.
 *
 * This example logs the send event. The application may be extended to
 * log timestamps or measure round-trip latency.
 */
void OnSendCallback(XBee* self, void* data) {
    portDebugPrintf("[TX] Send callback invoked.\n");
    (void)self;
    (void)data;
}

/**
 * Main entry point for the cellular socket example.
 *
 * This function:
 * - Initializes the UART and the XBeeCellular instance.
 * - Configures the cellular network (APN).
 * - Connects to the network.
 * - Opens a TCP socket to numbersapi.com.
 * - Sends a GET /random/trivia HTTP request.
 * - Waits for incoming responses over 15 seconds, or exits early if received.
 */
int main() {
    // Hardware abstraction setup
    const XBeeHTable hw = {
        .PortUartInit  = portUartInit,
        .PortUartRead  = portUartRead,
        .PortUartWrite = portUartWrite,
        .PortFlushRx   = portFlushRx,
        .PortDelay     = portDelay,
        .PortMillis    = portMillis,
    };

    // User callbacks for RX/TX events
    const XBeeCTable cb = {
        .OnReceiveCallback = OnReceiveCallback,
        .OnSendCallback = OnSendCallback
    };

    portDebugPrintf("XBee 3 Cellular - HTTP GET Example\n");

    // Allocate instance
    XBeeCellular* xbee = XBeeCellularCreate(&cb, &hw);

    // Initialize serial port
    if (!XBeeInit((XBee*)xbee, 9600, DEFAULT_SERIAL_PORT)) {
        portDebugPrintf("[ERR] Failed to initialize UART\n");
        return -1;
    }

    // Set SIM configuration: APN is required, others optional
    XBeeCellularConfig_t cfg = {
        .apn = "broadband",  
        .simPin = "",
        .carrier = ""
    };
    XBeeConfigure((XBee*)xbee, &cfg);

    // Attach to cellular network
    portDebugPrintf("Connecting to LTE network...\n");
    XBeeConnect((XBee*)xbee, false);

    while (1) {
        if (XBeeCellularConnected((XBee*)xbee)) {
            portDebugPrintf("[OK] Connected to cellular network.\n");
            break;
        }
        portDebugPrintf("Waiting for network attach...\n");
        portDelay(1000);
    }

    // Create a TCP socket
    uint8_t socketId = 0;
    if (!XBeeCellularSocketCreate((XBee*)xbee, 0x01 /* TCP */, &socketId)) {
        portDebugPrintf("[ERR] Socket create failed\n");
        return -1;
    }

    // Connect socket to numbersapi.com
    if (!XBeeCellularSocketConnect((XBee*)xbee, socketId, "numbersapi.com", 80, true)) {
        portDebugPrintf("[ERR] Socket connect failed\n");
        return -1;
    }

    // Send HTTP GET /random/trivia
    const char* httpRequest =
        "GET /random/trivia HTTP/1.1\r\n"
        "Host: numbersapi.com\r\n"
        "Connection: close\r\n\r\n";

    if (!XBeeCellularSocketSend((XBee*)xbee, socketId,
            (const uint8_t*)httpRequest, strlen(httpRequest))) {
        portDebugPrintf("[ERR] Socket send failed\n");
        return -1;
    }

    portDebugPrintf("[OK] HTTP GET request sent. Awaiting response...\n");

    // Wait up to 15 seconds for HTTP response, exit early if received
    uint32_t start = portMillis();
    while ((portMillis() - start) < 15000) {
        XBeeProcess((XBee*)xbee);
        if (responseReceived) {
            portDebugPrintf("[OK] Response received. Exiting wait early.\n");
            break;
        }
        portDelay(500);
    }

    portDebugPrintf("HTTP transaction complete. Exiting.\n");
    XBeeCellularSocketClose((XBee*)xbee, socketId, false);
    XBeeCellularDestroy(xbee);
    return 0;
}