/**
 * @file xbee_lr.c
 * @brief Implementation of XBee LR (LoRaWAN) subclass.
 * 
 * This file contains the implementation of functions specific to the XBee LR module.
 * It includes methods for initializing, sending join requests, and handling other
 * operations unique to the XBee LR subclass.
 * 
 * @version 1.2
 * @date 2025-05-01
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

 #include "xbee_lr.h"
 #include "xbee_api_frames.h"
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 
 static void SendJoinReqApiFrame(XBee* self);
 
 // XBeeLR specific implementations
 
 
 /**
  * @brief Checks if the XBee LR module is connected to the LoRaWAN network.
  * 
  * This function sends an AT command (`AT_JS`) to the XBee LR module to query the 
  * Join Status, determining whether the module is currently connected to the LoRaWAN network. 
  * It returns true if the module is connected (i.e., has joined the network) and false otherwise. 
  * The function also handles the communication with the module and provides debug output in case 
  * of communication errors.
  * 
  * @param[in] self Pointer to the XBee instance.
  * 
  * @return bool Returns true if the XBee LR module is connected to the network, otherwise false.
  */
 bool XBeeLRConnected(XBee* self) {
     // Implement logic to check XBeeLR network connection
     uint8_t response = 0;
     uint8_t responseLength;
     int status;
 
     // Send the AT_JS command to query the Join Status
     status = apiSendAtCommandAndGetResponse(self, AT_JS, NULL, 0, &response, &responseLength, 5000, sizeof(response));
 
     if (status == API_SEND_SUCCESS) {
         // Print the received reponse
         // XBEEDebugPrint("ATJS Resp: %u \n", response);
         // XBEEDebugPrint("Join Status: %s \n", response ? "Joined" : "Not Joined");
     } else {
         XBEEDebugPrint("Failed to receive AT_JS response, error code: %d\n", status);
     }
     return response;  
 }
 
 /**
  * @brief Initializes the XBee LR module for communication.
  * 
  * This function initializes the XBee LR module by setting up the necessary 
  * serial communication parameters, such as the baud rate and device path. 
  * It relies on platform-specific UART initialization provided by the hardware 
  * abstraction layer. The function returns true if initialization is successful.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] baudrate The baud rate for serial communication.
  * @param[in] device The path to the serial device (e.g., "/dev/ttyUSB0").
  * 
  * @return bool Returns true if the initialization is successful, otherwise false.
  */
 bool XBeeLRInit(XBee* self, uint32_t baudRate, void* device) {
     // Implement XBeeLR initialization
     return (self->htable->PortUartInit(baudRate, device)) == UART_SUCCESS ? true:false;
 }
 
 /**
  * @brief Processes incoming data and events for the XBee LR module.
  * 
  * This function must be called continuously in the main loop of the application. 
  * It handles the reception and processing of API frames from the XBee LR module.
  * The function checks for incoming frames, and if a frame is successfully received,
  * it is processed accordingly. 
  * 
  * @param[in] self Pointer to the XBee instance.
  * 
  * @return void This function does not return a value.
  */
 void XBeeLRProcess(XBee* self) {
     // Implement XBeeLR specific process logic
     xbee_api_frame_t frame;
     int status = apiReceiveApiFrame(self,&frame);
     if (status == API_SEND_SUCCESS) {
         apiHandleFrame(self,frame);
     } else if (status != API_RECEIVE_ERROR_TIMEOUT_START_DELIMITER) {
         XBEEDebugPrint("Error receiving frame.\n");
     }
 }
 
 
 /**
  * @brief Attempts to connect to the LoRaWAN network using the XBee LR module.
  * 
  * This function initiates the connection process to a LoRaWAN network by sending 
  * a join request. The function supports both blocking and non-blocking operation.
  * If `blocking` is true, it waits until the connection attempt is completed or times out.
  * If `blocking` is false, the function returns immediately after initiating the join.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] blocking If true, waits until join completes or times out. Otherwise returns immediately.
  * 
  * @return bool Returns true if the connection was successful (in blocking mode) 
  *         or if the request was initiated (in non-blocking mode).
  */
bool XBeeLRConnect(XBee* self, bool blocking) {
    XBEEDebugPrint("Join Request Sent...\n");
    SendJoinReqApiFrame(self);

    if (!blocking) {
        return true; // Return immediately in non-blocking mode
    }

    // Start the timeout timer
    uint32_t startTime = portMillis();

    // Delay until CONNECTION_TIMEOUT_MS time has elapsed
    while ((portMillis() - startTime) < CONNECTION_TIMEOUT_MS) {}

    XBEEDebugPrint("Checking Join Status...\n");

    // Check Join Status only once per Join Request
    if (XBeeLRConnected(self)) {
        XBEEDebugPrint("Successfully Joined\n");
        return true; // Successfully joined
    }

    portDelay(500); // Delay between checks
    XBEEDebugPrint("Failed to Join\n");
    return false; // Timeout reached without successful join
}
 
 /**
  * @brief Disconnects from the LoRaWAN network using the XBee LR module.
  * 
  * This function handles the disconnection process from a LoRaWAN network. It ensures 
  * that the XBee LR module is properly disconnected and that any necessary cleanup 
  * is performed. The function is currently blocking.
  * 
  * @param[in] self Pointer to the XBee instance.
  * 
  * @return bool Returns true if the disconnection process was initiated.
  */
 bool XBeeLRDisconnect(XBee* self) {
     // Implement XBeeLR specific disconnection logic
     (void) self;
     return true;
 }
 
 /**
  * @brief Sends data over the network using the XBee LR module.
  * 
  * This function constructs and sends a data packet over the network using an XBee LR module.
  * The function is currently blocking, meaning it waits until the data is fully transmitted
  * before returning. A future enhancement (@todo) could add support for non-blocking operation.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] data Pointer to the data to be sent, encapsulated in an XBeeLRPacket_t structure.
  * 
  * @return xbee_deliveryStatus_t, 0 if successful
  * 
  */
 uint8_t XBeeLRSendPacket(XBee* self, const void* data) {
     // Prepare and send the API frame
     XBeeLRPacket_t *packet = (XBeeLRPacket_t*) data;
     uint8_t frame_data[128];  // Adjust size as needed
     packet->frameId = self->frameIdCntr;
     frame_data[0] = self->frameIdCntr;
     frame_data[1] = packet->port;
     frame_data[2] = packet->ack & 0x01;
     memcpy(&frame_data[3], packet->payload, packet->payloadSize);
 
     // Send the frame
     int send_status = apiSendFrame(self, XBEE_API_TYPE_LR_TX_REQUEST, frame_data, 3 + packet->payloadSize);
     if (send_status != API_SEND_SUCCESS) {
         return false;  // Failed to send the frame
     }
 
     // Block and wait for the XBEE_API_TYPE_TX_STATUS frame
     uint32_t startTime = portMillis();  // Get the current time in milliseconds
 
     self->txStatusReceived = false;  // Reset the status flag before waiting
 
     while ((portMillis() - startTime) < SEND_DATA_TIMEOUT_MS) {
         // Process incoming frames using XBeeLRProcess
         XBeeLRProcess(self);
 
         // Check if the status frame was received
         if (self->txStatusReceived) {
             // Return the delivery status
             if(self->deliveryStatus){
                 XBEEDebugPrint("TX Delivery Status 0x%02X\n", self->deliveryStatus );
             }
             return self->deliveryStatus;
         }
 
         // Add a small delay here to avoid busy-waiting
         portDelay(10);  // Delay for 10 ms
     }
 
     // Timeout reached without receiving the expected frame
     XBEEDebugPrint("Failed to receive TX Request Status frame\n");
     return 0xFF;  // Indicate failure or timeout
 }
 
 bool XBeeLRSoftReset(XBee* self) {
     // Implement XBeeLR specific soft reset logic
     (void) self;
     return true;
 }
 
 void XBeeLRHardReset(XBee* self) {
    (void) self;
     // Implement XBeeLR specific hard reset logic
 }
 
 /* XBeeLR Specific Functions */
 
 /**
  * @brief Sends the AT_AE command to set the LoRaWAN AppEUI on the XBee LR module.
  * 
  * This function configures the LoRaWAN AppEUI (Application Identifier) on the XBee LR module 
  * by sending the AT command `AT_AE` with the specified AppEUI value. The function is blocking, 
  * meaning it waits for a response from the module or until a timeout occurs. If the command 
  * fails to send or the module does not respond, a debug message is printed.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The AppEUI to be set, provided as a string.
  * 
  * @return bool Returns true if the AppEUI was successfully set, otherwise false.
  */
 bool XBeeLRSetAppEUI(XBee* self, const char* value) {
     uint8_t response[17];
     uint8_t responseLength;
     uint8_t param[8];
     
     if (!value || strlen(value) != 16) {
         XBEEDebugPrint("Invalid App EUI length\n");
         return false;
     }
     
     if (asciiToHexArray(value, param, sizeof(param)) < 0) {
         XBEEDebugPrint("Failed to convert App EUI\n");
         return false;
     }
     
     int status = apiSendAtCommandAndGetResponse(self, AT_AE, param, sizeof(param), response, &responseLength, 5000, sizeof(response));
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set App EUI\n");
         return false;
     }
     return true;
 }
 
 /**
  * @brief Sends the AT_AK command to set the LoRaWAN AppKey on the XBee LR module.
  * 
  * This function configures the LoRaWAN AppKey (Application Key) on the XBee LR module 
  * by sending the AT command `AT_AK` with the specified AppKey value. The function is 
  * blocking, meaning it waits for a response from the module or until a timeout occurs. 
  * If the command fails to send or the module does not respond, a debug message is printed.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The AppKey to be set, provided as a string.
  * 
  * @return bool Returns true if the AppKey was successfully set, otherwise false.
  */
 bool XBeeLRSetAppKey(XBee* self, const char* value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t param[16];
     
     if (!value || strlen(value) != 32) {
         XBEEDebugPrint("Invalid App Key length %d\n", strlen(value));
         return false;
     }
     
     if (asciiToHexArray(value, param, sizeof(param)) < 0) {
         XBEEDebugPrint("Failed to convert App Key\n");
         return false;
     }
     
     int status = apiSendAtCommandAndGetResponse(self, AT_AK, param, sizeof(param), response, &responseLength, 5000, sizeof(response));
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set App Key\n");
         return false;
     }
     return true;
 }
 
 /**
  * @brief Sends the AT_NK command to set the LoRaWAN NwkKey on the XBee LR module.
  * 
  * This function configures the LoRaWAN NwkKey (Network Key) on the XBee LR module 
  * by sending the AT command `AT_NK` with the specified NwkKey value. The function 
  * is blocking, meaning it waits for a response from the module or until a timeout 
  * occurs. If the command fails to send or the module does not respond, a debug 
  * message is printed.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The NwkKey to be set, provided as a string.
  * 
  * @return bool Returns true if the NwkKey was successfully set, otherwise false.
  */
 bool XBeeLRSetNwkKey(XBee* self, const char* value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t param[16];
     
     if (!value || strlen(value) != 32) {
         XBEEDebugPrint("Invalid Nwk Key length %d\n", strlen(value));
         return false;
     }
     
     if (asciiToHexArray(value, param, sizeof(param)) < 0) {
         XBEEDebugPrint("Failed to convert Nwk Key\n");
         return false;
     }
     
     int status = apiSendAtCommandAndGetResponse(self, AT_NK, param, sizeof(param), response, &responseLength, 5000, sizeof(response));
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Nwk Key\n");
         return false;
     }
     return true;
 }
 
 /**
  * @brief Sends the AT_LC command to set the LoRaWAN Class on the XBee LR module.
  * 
  * This function configures the LoRaWAN Class on the XBee LR module 
  * by sending the AT command `AT_LC` with the specified Class value. The function 
  * is blocking, meaning it waits for a response from the module or until a timeout 
  * occurs. If the command fails to send or the module does not respond, a debug 
  * message is printed.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Class to be set, provided as a char. 'A','B','C'
  * 
  * @return bool Returns true if the Class was successfully set, otherwise false.
  */
 bool XBeeLRSetClass(XBee* self, const char value) {
     uint8_t response[33];
     uint8_t responseLength;
     int status = apiSendAtCommandAndGetResponse(self, AT_LC, (const uint8_t *) &value, 1, response, &responseLength, 5000, sizeof(response));
     if(status != API_SEND_SUCCESS){
         XBEEDebugPrint("Failed to set Class\n");
         return false;
     }
     return true;
 }
 
 /**
  * @brief Sends the AT_AM command to set the LoRaWAN Activation Mode on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Activation Mode to be set.
  * 
  * @return bool Returns true if the Activation Mode was successfully set, otherwise false.
  */
 bool XBeeLRSetActivationMode(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
 
     int status = apiSendAtCommandAndGetResponse(self, AT_AM, &value, 1, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Activation Mode\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_AD command to set the LoRaWAN ADR on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The ADR setting to be set.
  * 
  * @return bool Returns true if the ADR was successfully set, otherwise false.
  */
 bool XBeeLRSetADR(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
 
     int status = apiSendAtCommandAndGetResponse(self, AT_AD, &value, 1, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set ADR\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_DR command to set the LoRaWAN DataRate on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The DataRate to be set.
  * 
  * @return bool Returns true if the DataRate was successfully set, otherwise false.
  */
 bool XBeeLRSetDataRate(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
 
     int status = apiSendAtCommandAndGetResponse(self, AT_DR, &value, 1, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set DataRate\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_LR command to set the LoRaWAN Region on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Region to be set.
  * 
  * @return bool Returns true if the Region was successfully set, otherwise false.
  */
 bool XBeeLRSetRegion(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
 
     int status = apiSendAtCommandAndGetResponse(self, AT_LR, &value, 1, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Region\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_DC command to set the LoRaWAN Duty Cycle on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Duty Cycle to be set.
  * 
  * @return bool Returns true if the Duty Cycle was successfully set, otherwise false.
  */
 bool XBeeLRSetDutyCycle(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
 
     int status = apiSendAtCommandAndGetResponse(self, AT_DC, &value, 1, response, &responseLength, 5000,sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Duty Cycle\n");
         return false;
     }
 
     return true;
 }
 
 // /**
 //  * @brief Sends the AT_LV command to get the LoRaWAN Spec Version on the XBee LR module.
 //  * 
 //  * @param[in] self Pointer to the XBee instance.
 //  * @param[in] value The Spec Version to be set.
 //  * 
 //  * @return bool Returns true if the Spec Version was successfully set, otherwise false.
 //  */
 // bool XBeeLRGetSpecVersion(XBee* self, const char value) {
 //     uint8_t response[33];
 //     uint8_t responseLength;
 //     uint8_t paramLength = 1;
 
 //     int status = apiSendAtCommandAndGetResponse(self, AT_LV, &value, paramLength, response, &responseLength, 5000);
 
 //     if (status != API_SEND_SUCCESS) {
 //         XBEEDebugPrint("Failed to set Spec Version\n");
 //         return false;
 //     }
 
 //     return true;
 // }
 
 /**
  * @brief Sends the AT_J1 command to set the LoRaWAN Join RX1 Delay on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Join RX1 Delay to be set.
  * 
  * @return bool Returns true if the Join RX1 Delay was successfully set, otherwise false.
  */
 bool XBeeLRSetJoinRX1Delay(XBee* self, const uint32_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_J1,(const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Join RX1 Delay\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_J2 command to set the LoRaWAN Join RX2 Delay on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Join RX2 Delay to be set.
  * 
  * @return bool Returns true if the Join RX2 Delay was successfully set, otherwise false.
  */
 bool XBeeLRSetJoinRX2Delay(XBee* self, const uint32_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_J2, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Join RX2 Delay\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_D1 command to set the LoRaWAN RX1 Delay on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The RX1 Delay to be set.
  * 
  * @return bool Returns true if the RX1 Delay was successfully set, otherwise false.
  */
 bool XBeeLRSetRX1Delay(XBee* self, const uint32_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_D1, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set RX1 Delay\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_D2 command to set the LoRaWAN RX2 Delay on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The RX2 Delay to be set.
  * 
  * @return bool Returns true if the RX2 Delay was successfully set, otherwise false.
  */
 bool XBeeLRSetRX2Delay(XBee* self, const uint32_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_D2, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set RX2 Delay\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_XD command to set the LoRaWAN RX2 Data Rate on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The RX2 Data Rate to be set.
  * 
  * @return bool Returns true if the RX2 Data Rate was successfully set, otherwise false.
  */
 bool XBeeLRSetRX2DataRate(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self,AT_XD, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set RX2 Data Rate\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_XF command to set the LoRaWAN RX2 Frequency on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The RX2 Frequency to be set.
  * 
  * @return bool Returns true if the RX2 Frequency was successfully set, otherwise false.
  */
 bool XBeeLRSetRX2Frequency(XBee* self, const uint32_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_XF, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set RX2 Frequency\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_PO command to set the LoRaWAN Transmit Power on the XBee LR module.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value The Transmit Power to be set.
  * 
  * @return bool Returns true if the Transmit Power was successfully set, otherwise false.
  */
 bool XBeeLRSetTransmitPower(XBee* self, const uint8_t value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t paramLength = sizeof(value);
 
     int status = apiSendAtCommandAndGetResponse(self, AT_PO, (const uint8_t*)&value, paramLength, response, &responseLength, 5000, sizeof(response));
 
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Transmit Power\n");
         return false;
     }
 
     return true;
 }
 
 /**
  * @brief Sends the AT_DE command to read the LoRaWAN DevEUI from the XBee LR module.
  * 
  * This function retrieves the LoRaWAN DevEUI (Device Extended Unique Identifier) 
  * from the XBee LR module by sending the AT command `AT_DE`. The function is 
  * blocking, meaning it waits for a response from the module or until a timeout occurs. 
  * If the command fails to send or the module does not respond, a debug message is printed. 
  * The DevEUI is stored in the provided response buffer.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[out] responseBuffer Buffer to store the retrieved DevEUI.
  * @param[in] buffer_size Size of the response buffer (should be at least 17 bytes).
  * 
  * @return bool Returns true if the DevEUI was successfully retrieved, otherwise false.
  */
 bool XBeeLRGetDevEUI(XBee* self, char* responseBuffer, uint8_t buffer_size) {
     // Ensure buffer is large enough to hold DevEUI (16 ASCII characters + null terminator)
     if (buffer_size < 17) {
         return false;
     }
     memset(responseBuffer, 0, buffer_size);
 
     // Send the AT_DE command to query the DevEUI
     uint8_t rawResponse[8]; // DevEUI is 8 bytes in binary
     uint8_t responseLength;
     int status = apiSendAtCommandAndGetResponse(self, AT_DE, NULL, 0, rawResponse, &responseLength, 5000, sizeof(rawResponse));
 
     if (status != API_SEND_SUCCESS || responseLength != 8) {
         XBEEDebugPrint("Failed to receive valid AT_DE response, error code: %d\n", status);
         return false;
     }
 
     // Convert binary DevEUI to ASCII string representation
     for (int i = 0; i < 8; i++) {
         snprintf(&responseBuffer[i * 2], 3, "%02X", rawResponse[i]);
     }
 
     return true;
 }
 
 
 /**
  * @brief Sends the AT_CM command to set the LoRaWAN Channels Mask on the XBee LR module.
  * 
  * This function configures the Channels Mask for the LoRaWAN network on the XBee LR module by
  * sending the AT_CM command. The Channels Mask specifies the sub-bands that are enabled for 
  * communication. The function checks the command response to verify if the mask was successfully set.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] value Pointer to a null-terminated string representing the Channels Mask in hexadecimal format.
  * 
  * @return bool Returns true if the Channels Mask was successfully set; otherwise, false.
  */
 bool XBeeLRSetChannelsMask(XBee* self, const char* value) {
     uint8_t response[33];
     uint8_t responseLength;
     uint8_t param[16]; // Maximum LoRaWAN channel mask size
     size_t paramLength = strlen(value) / 2;
     
     if (!value || strlen(value) % 2 != 0 || paramLength > sizeof(param)) {
         XBEEDebugPrint("Invalid Channels Mask length\n");
         return false;
     }
     
     if (asciiToHexArray(value, param, paramLength) < 0) {
         XBEEDebugPrint("Failed to convert Channels Mask\n");
         return false;
     }
     
     int status = apiSendAtCommandAndGetResponse(self, AT_CM, param, paramLength, response, &responseLength, 5000, sizeof(response));
     if (status != API_SEND_SUCCESS) {
         XBEEDebugPrint("Failed to set Channels Mask\n");
         return false;
     }
     return true;
 }
 
 
 // XBeeLR private functions
 
 /**
  * @brief Sends a join request API frame.
  * 
  * This function sends a join request API frame to the XBee network,
  * which is part of the joining process for LR modules.
  * 
  * @param[in] self Pointer to the XBee instance.
  */
 static void SendJoinReqApiFrame(XBee* self) {
     uint8_t frame_id = self->frameIdCntr;
 
     // Call the api_send_frame function to send the Join Request API frame
     apiSendFrame(self, XBEE_API_TYPE_LR_JOIN_REQUEST, &frame_id, 1);
 }
 
 /**
  * @brief Parses an RX_PACKET frame and invokes the receive callback function.
  * 
  * This function processes a received RX_PACKET frame specific to the XBee LR module. 
  * It parses the frame data, extracting relevant information such as the port number, 
  * RSSI, SNR, and payload. If the frame is of type `XBEE_API_TYPE_LR_RX_PACKET` or 
  * `XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET`, the parsed data is stored in an `XBeeLRPacket_t` 
  * structure, and the receive callback function (`OnReceiveCallback`) is called with this 
  * data. The function ensures that the callback is only invoked if the receive data is valid.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] param Pointer to the received API frame data.
  * 
  * @return void This function does not return a value.
  */
 static void XBeeLRHandleRxPacket(XBee* self, void *param) {
 
     if (param == NULL) return;
 
     xbee_api_frame_t *frame = (xbee_api_frame_t *)param;
     if (frame->type != XBEE_API_TYPE_LR_RX_PACKET && frame->type != XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET) return;
 
     XBeeLRPacket_t packet = {0}; // Allocate on the stack and zero-initialize
 
     APIFrameDebugPrint("RX Packet Data: ");
     for (int i = 0; i < frame->length; i++) {
         APIFrameDebugPrint("0x%02X ", frame->data[i]);
     }
     APIFrameDebugPrint("\n");
 
     if (frame->type == XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET) {
         packet.port = frame->data[1];
         packet.rssi = frame->data[2];
         packet.snr = frame->data[3];
         packet.dr = frame->data[4] & 0xF;
         packet.slot = frame->data[4] >> 4;
         packet.counter = frame->data[5] << 24 | frame->data[6] << 16 | frame->data[7] << 8 | frame->data[8];
         packet.payloadSize = frame->length - 10;
         packet.payload = &(frame->data[10]); // Point directly to the payload in the frame data
     } else {
         packet.port = frame->data[1];
         packet.payloadSize = frame->length - 2;
         packet.payload = &(frame->data[2]); // Point directly to the payload in the frame data
     }
 
     if (self->ctable->OnReceiveCallback) {
         self->ctable->OnReceiveCallback(self, &packet); // Pass the address of the stack variable
     }
 }
 
 
 void XBeeLRHandleTransmitStatus(XBee* self, void *param) {
 
     if (param == NULL) return;
 
     xbee_api_frame_t *frame = (xbee_api_frame_t *)param;
     if (frame->type != XBEE_API_TYPE_TX_STATUS && frame->type != XBEE_API_TYPE_LR_EXPLICIT_TX_STATUS) return;
 
     XBeeLRPacket_t packet = {0}; // Allocate on the stack and zero-initialize
 
     APIFrameDebugPrint("Received Transmit Status Frame: ");
     for (int i = 1; i < frame->length; i++) {
         APIFrameDebugPrint("0x%02X ", frame->data[i]);
     }
     APIFrameDebugPrint("\n");
 
     packet.frameId = frame->data[1];
     packet.status = frame->data[2];
 
     // Store the delivery status in the XBee instance
     self->deliveryStatus = frame->data[2];  // Extract Delivery Status
 
     if (frame->type == XBEE_API_TYPE_LR_EXPLICIT_TX_STATUS) {
         packet.dr = frame->data[3];
         packet.channel = frame->data[4];
         packet.power = frame->data[5];
         packet.counter = frame->data[6] << 24 | frame->data[7] << 16 | frame->data[8] << 8 | frame->data[9];
     }
 
     // Set the txStatusReceived flag to indicate the status frame was received
     self->txStatusReceived = true;
 
     if (self->ctable->OnSendCallback) {
         self->ctable->OnSendCallback(self, &packet); // Pass the address of the stack variable
     }
 
 }
 
 
 // VTable for XBeeLR
 const XBeeVTable XBeeLRVTable = {
     .init = XBeeLRInit,
     .process = XBeeLRProcess,
     .connect = XBeeLRConnect,
     .disconnect = XBeeLRDisconnect,
     .sendData = XBeeLRSendPacket,
     .softReset = XBeeLRSoftReset,
     .hardReset = XBeeLRHardReset,
     .connected = XBeeLRConnected,
     .handleRxPacketFrame = XBeeLRHandleRxPacket,
     .handleTransmitStatusFrame = XBeeLRHandleTransmitStatus,
 };
 
 /**
  * @brief Constructor for creating an XBeeLR instance.
  * 
  * This function allocates memory for a new XBeeLR instance and initializes it 
  * with the provided callback table (`cTable`) and handler table (`hTable`). 
  * The function sets up the virtual table (`vtable`) for XBee LR-specific operations 
  * and assigns the callback and handler tables to the instance. The newly created 
  * instance is then returned to the caller.
  * 
  * @param[in] cTable Pointer to the callback table containing function pointers for handling XBee events.
  * @param[in] hTable Pointer to the handler table containing platform-specific function implementations.
  * 
  * @return XBeeLR* Pointer to the newly created XBeeLR instance.
  */
 XBeeLR* XBeeLRCreate(const XBeeCTable* cTable, const XBeeHTable* hTable) {
     XBeeLR* instance = (XBeeLR*)malloc(sizeof(XBeeLR));
     instance->base.vtable = &XBeeLRVTable;
     instance->base.htable = hTable;
     instance->base.ctable = cTable;
     return instance;
 }
 
 void XBeeLRDestroy(XBeeLR* self) {
     free(self);
 }
 