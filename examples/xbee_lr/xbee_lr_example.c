/***************************************************************************//**
 * @file xbee_lr_example.c
 * @brief Example application demonstrating the use of the XBee library.
 *
 * This file contains a sample application that demonstrates how to use the XBee library 
 * to communicate with XBee modules in a platform-agnostic environment. It showcases basic operations 
 * such as initializing the XBee module, connecting to the network, and transmitting & receiving data.
 *
 * @version 1.1
 * @date 2025-05-30
 * @author Felix Galindo
 * @contact felix.galindo@digi.com
 ******************************************************************************/

#include "xbee_lr.h"
#include "port.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// Choose default serial port path based on OS
#if defined(_WIN32)
    #define DEFAULT_SERIAL_PORT "COM3"
#elif defined(__APPLE__)
    #define DEFAULT_SERIAL_PORT "/dev/cu.usbserial-1110"
#elif defined(__linux__)
    #define DEFAULT_SERIAL_PORT "/dev/ttyUSB0"
#else
    #define DEFAULT_SERIAL_PORT ""
#endif

/**
 * @brief Callback function triggered when data is received from the XBee module.
 * 
 * This function is called when the XBee module receives data. It processes the 
 * incoming data, extracts relevant information, and handles it accordingly. 
 * The function is typically registered as a callback to be executed automatically 
 * upon data reception.
 * 
 * @param[in] frame Pointer to the received XBee API frame.
 * 
 * @return void This function does not return a value.
 */
void OnReceiveCallback(XBee* self, void* data){
    (void) self;
    XBeeLRPacket_t* packet = (XBeeLRPacket_t*) data;
    portDebugPrintf("Received Packet: ");
    for (int i = 0; i < packet->payloadSize; i++) {
        portDebugPrintf("0x%02X ", packet->payload[i]);
    }
    portDebugPrintf("\n");
    portDebugPrintf("Ack %u\n", packet->ack);
    portDebugPrintf("Port %u\n", packet->port);
    portDebugPrintf("RSSI %d\n", packet->rssi);
    portDebugPrintf("SNR %d\n", packet->snr);
    portDebugPrintf("Downlink Counter %lu\n", packet->counter);
}

/**
 * @brief Callback function triggered after data is sent from the XBee module.
 * 
 * This function is called when the XBee module completes sending data. It handles 
 * any post-send operations, such as logging the transmission status or updating 
 * the state of the application. The function is typically registered as a callback 
 * to be executed automatically after data is transmitted.
 * 
 * @param[in] frameId The ID of the API frame that was sent.
 * @param[in] status The status code indicating the result of the send operation.
 * 
 * @return void This function does not return a value.
 */
void OnSendCallback(XBee* self, void* data){
    (void) self;
    XBeeLRPacket_t* packet = (XBeeLRPacket_t*) data;
    switch(packet->status){
        case 0:
            portDebugPrintf("Send successful (frameId: 0x%02X)\n",packet->frameId);
            break;
        case 0x01:
            portDebugPrintf("Send failed (frameId: 0x%02X) (reason: Ack Failed)\n",packet->frameId);
            break;
        case 0x022:
            portDebugPrintf("Send failed (frameId: 0x%02X) (reason: Not Connected)\n",packet->frameId);
            break;
        default:
            portDebugPrintf("Send failed (frameId: 0x%02X) (reason: 0x%02X)\n",packet->frameId, packet->status);
            break;
    }
}

int main() {

    // Harware Abstraction Function Pointer Table for XBeeLR (needs to be set!)
    const XBeeHTable XBeeLRHTable = {
        .PortUartRead = portUartRead,
        .PortUartWrite = portUartWrite,
        .PortMillis = portMillis,
        .PortFlushRx = portFlushRx,
        .PortUartInit = portUartInit,
        .PortDelay = portDelay,
    };

    // Callback Function Pointer Table for XBeeLR
    const XBeeCTable XBeeLRCTable = {
        .OnReceiveCallback = OnReceiveCallback, //can be left as all NULL if no callbacks needed
        .OnSendCallback = NULL, //can be left as all NULL if no callbacks needed
    };

    portDebugPrintf("XBee LR Example App\n");

    // Create an instance of the XBeeLR class
    XBeeLR * myXbeeLr = XBeeLRCreate(&XBeeLRCTable,&XBeeLRHTable);

    // Init XBee
    if(!XBeeInit((XBee*)myXbeeLr, 9600, DEFAULT_SERIAL_PORT)){
        portDebugPrintf("Failed to initialize XBee\n");
    }

    // Read LoRaWAN DevEUI and print
    char devEui[17];
    XBeeLRGetDevEUI((XBee*)myXbeeLr, devEui, sizeof(devEui));
    portDebugPrintf("DEVEUI: %s\n", devEui);

    // Set LoRaWAN Network Settings
    portDebugPrintf("Configuring...\n");
    XBeeLRSetAppEUI((XBee*)myXbeeLr, "9E1177BD6B1DF41E");
    XBeeLRSetAppKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeLRSetNwkKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeLRSetRegion((XBee*)myXbeeLr, 8);
    XBeeLRSetClass((XBee*)myXbeeLr, 'C');
    XBeeSetAPIOptions((XBee*)myXbeeLr, 0x01);
    XBeeWriteConfig((XBee*)myXbeeLr);
    XBeeApplyChanges((XBee*)myXbeeLr);

    // Connect to LoRaWAN network
    portDebugPrintf("Connecting...\n");
    bool connected = false;
    if(XBeeConnect((XBee*)myXbeeLr,true)){connected = true;}

    // XBeeLR payload to send
    uint8_t examplePayload[] = {0xC0, 0xC0, 0xC0, 0xFF, 0xEE};
    uint16_t payloadLen = sizeof(examplePayload) / sizeof(examplePayload[0]);
    XBeeLRPacket_t packet = {
        .payload = examplePayload,
        .payloadSize = payloadLen,
        .port = 2,
        .ack = 0,
    };

    // Use portMillis for time tracking
    uint32_t startMs = portMillis();

    while (1) {
        // Let XBee class process any serial data
        XBeeProcess((XBee*)myXbeeLr);

        // Get current time in milliseconds
        uint32_t currentMs = portMillis();

        // Check if 10 seconds have passed
        if ((currentMs - startMs) >= 10000) {
            // Send data if connected, else connect
            if(connected){
                portDebugPrintf("Sending 0x");
                for (int i = 0; i < payloadLen; i++) {
                    portDebugPrintf("%02X", examplePayload[i]);
                }
                portDebugPrintf("\n");
                if (XBeeSendPacket((XBee*)myXbeeLr, &packet)) {
                    printf("Failed to send data.\n");
                } else {
                    printf("Data sent successfully.\n");
                }

                // Update payload
                packet.payload[0] = packet.payload[0] + 1; // change payload
            }
            else{
                portDebugPrintf("Not connected. Connecting...\n");
                if (!XBeeConnect((XBee*)myXbeeLr, true)) {
                    printf("Failed to connect.\n");
                } else {
                    connected = true;
                    printf("Connected!\n");
                }
            }
            // Reset the start time
            startMs = portMillis();
        }

        // Delay 100 ms between loops
        portDelay(100);
    }

    return 0;
}