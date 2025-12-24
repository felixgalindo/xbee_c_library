/**
 * @file xbee_api_frames.c
 * @brief Implementation of XBee API frame handling.
 * 
 * This file contains the implementation of functions used to create, parse, 
 * and handle API frames for XBee communication. API frames are the primary 
 * method for structured data exchange with XBee modules.
 * 
 * @version 1.1
 * @date 2025-04-04
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

 #include "xbee_api_frames.h"
 #include "xbee.h"
 #include "port.h"
 #include <stdio.h>
 #include <string.h>
 
 // API Frame Functions
 
 /**
  * @brief Calculates the checksum for an API frame.
  * 
  * This function computes the checksum for a given XBee API frame. The checksum 
  * is calculated by summing the bytes of the frame starting from the fourth byte 
  * (index 3) to the end of the frame and then subtracting the sum from 0xFF. 
  * The resulting checksum ensures the integrity of the data in the API frame.
  * 
  * @param[in] frame Pointer to the API frame data.
  * @param[in] len Length of the API frame data.
  * 
  * @return uint8_t The calculated checksum value.
  */
 static uint8_t calculateChecksum(const uint8_t *frame, uint16_t len) {
     uint8_t sum = 0;
     for (uint16_t i = 3; i < len; i++) {
         sum += frame[i];
     }
     return 0xFF - sum;
 }
 
 /**
  * @brief Sends an XBee API frame.
  * 
  * This function constructs and sends an XBee API frame over the UART. The frame 
  * includes a start delimiter, length, frame type, data, and a checksum to ensure 
  * data integrity. The function increments the frame ID counter with each call, 
  * ensuring that frame IDs are unique. If the frame is successfully sent, the function 
  * returns 0; otherwise, it returns an error code indicating the failure.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] frameType The type of the API frame to send.
  * @param[in] data Pointer to the frame data to be included in the API frame.
  * @param[in] len Length of the frame data in bytes.
  * 
  * @return int Returns 0 (`API_SEND_SUCCESS`) if the frame is successfully sent, 
  * or a non-zero error code (`API_SEND_ERROR_UART_FAILURE`) if there is a failure.
  */
 int apiSendFrame(XBee* self, uint8_t frameType, const uint8_t *data, uint16_t len) {
     uint8_t frame[256];
     uint16_t frameLength = 0;
     self->frameIdCntr++;
     if (self->frameIdCntr == 0) self->frameIdCntr = 1; // Reset frame counter when 0
 
     // Start delimiter
     frame[frameLength++] = 0x7E;
 
     // Length MSB and LSB
     frame[frameLength++] = (len + 1) >> 8;
     frame[frameLength++] = (len + 1) & 0xFF;
 
     // Frame type
     frame[frameLength++] = frameType;
 
     // Frame data
     memcpy(&frame[frameLength], data, len);
     frameLength += len;
 
     // Calculate and add checksum
     frame[frameLength] = calculateChecksum(frame, frameLength);
     frameLength++;
 
     // Print the API frame in hex format
     APIFrameDebugPrint("Sending API Frame: ");
     for (uint16_t i = 0; i < frameLength; i++) {
         APIFrameDebugPrint("0x%02X ", frame[i]);
     }
     APIFrameDebugPrint("\n");
 
     // Measure the time taken to send the frame
     uint32_t startTime = self->htable->PortMillis();
     int totalBytesWritten = 0;
 
     while (totalBytesWritten < frameLength) {
         int bytes_written = self->htable->PortUartWrite(frame + totalBytesWritten, frameLength - totalBytesWritten);
         if (bytes_written < 0) {
             return API_SEND_ERROR_UART_FAILURE;
         }
 
         totalBytesWritten += bytes_written;
 
         // Check for timeout
         if ((self->htable->PortMillis() - startTime) > UART_WRITE_TIMEOUT_MS) {
             APIFrameDebugPrint("Error: Frame sending timeout after %lu ms\n", self->htable->PortMillis() - startTime);
             return API_SEND_ERROR_UART_FAILURE;
         }
         self->htable->PortDelay(1);
     }
 
 #if API_FRAME_DEBUG_PRINT_ENABLED
     uint32_t elapsed_time = self->htable->PortMillis() - startTime;
 #endif
     APIFrameDebugPrint("UART write completed in %lu ms\n", elapsed_time);
 
     // Return success if everything went well
     return API_SEND_SUCCESS;
 }
 
 
 /**
  * @brief Sends an AT command through an API frame.
  * 
  * This function constructs and sends an AT command in API frame mode. It prepares 
  * the frame by including the frame ID, the AT command, and any optional parameters. 
  * The function checks for various errors, such as invalid commands or parameters 
  * that are too large, and returns appropriate error codes. If the AT command is 
  * successfully sent, the function returns 0.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] command The AT command to be sent, specified as an `at_command_t` enum.
  * @param[in] parameter Pointer to the parameter data to be included with the AT command (can be NULL).
  * @param[in] paramLength Length of the parameter data in bytes (0 if no parameters).
  * 
  * @return int Returns 0 (`API_SEND_SUCCESS`) if the AT command is successfully sent, 
  * or a non-zero error code if there is a failure (`API_SEND_ERROR_FRAME_TOO_LARGE`, 
  * `API_SEND_ERROR_INVALID_COMMAND`, etc.).
  */
 int apiSendAtCommand(XBee* self,at_command_t command, const uint8_t *parameter, uint8_t paramLength) {
     uint8_t frame_data[128];
     uint16_t frameLength = 0;
 
    // Check if the parameter length is too large
     if (paramLength > 128) {
         return API_SEND_ERROR_FRAME_TOO_LARGE;
     }
 
     // Frame ID
     frame_data[frameLength++] = self->frameIdCntr;
 
     // AT Command (2 bytes)
     const char *cmd_str = atCommandToString(command);
 
     if (cmd_str == NULL) {
         return API_SEND_ERROR_INVALID_COMMAND;
     }
     
     frame_data[frameLength++] = cmd_str[0];
     frame_data[frameLength++] = cmd_str[1];
 
     // AT Command Parameter
     if (paramLength > 0) {
         memcpy(&frame_data[frameLength], parameter, paramLength);
         frameLength += paramLength;
     }
 
     // Print the AT command and parameter in a readable format
     APIFrameDebugPrint("Sending AT Command: %s\n", cmd_str);
     if (paramLength > 0) {
         APIFrameDebugPrint("Parameter: ");
         for (uint8_t i = 0; i < paramLength; i++) {
             APIFrameDebugPrint("0x%02X ", parameter[i]);
         }
         APIFrameDebugPrint("\n");
     } else {
         APIFrameDebugPrint("No Parameters\n");
     }
 
     // Use api_send_frame to send the complete frame
     return apiSendFrame(self, XBEE_API_TYPE_AT_COMMAND, frame_data, frameLength);
 }
 
 /**
  * @brief Reads a specified number of bytes from the UART with a timeout mechanism.
  * 
  * This function attempts to read a specified number of bytes from the UART interface associated with
  * the XBee instance. It continuously reads data until the requested length is received or a timeout occurs.
  * If the UART read operation fails or the timeout is exceeded, the function returns an appropriate error code.
  * 
  * @param[in] self Pointer to the XBee instance, which contains the UART hardware table and other settings.
  * @param[out] buffer Pointer to the buffer where the received bytes will be stored.
  * @param[in] length The number of bytes to read from the UART.
  * @param[in] timeoutMs The maximum time in milliseconds to wait for the complete data to be read.
  * 
  * @return api_receive_status_t Returns API_RECEIVE_SUCCESS if the specified number of bytes are successfully read.
  *         Returns API_RECEIVE_ERROR_UART_FAILURE if the UART read operation fails.
  *         Returns API_RECEIVE_ERROR_TIMEOUT_DATA if the timeout is exceeded before the required bytes are read.
  */
 static api_receive_status_t readBytesWithTimeout(XBee* self, uint8_t* buffer, int length, uint32_t timeoutMs) {
     int totalBytesReceived = 0;
     int bytes_received = 0;
     uint32_t startTime = self->htable->PortMillis();
 
     while (totalBytesReceived < length) {
         bytes_received = self->htable->PortUartRead(buffer + totalBytesReceived, length - totalBytesReceived);
         
         if (bytes_received > 0) {
             totalBytesReceived += bytes_received;
         }
 
         // Check for timeout
         if (self->htable->PortMillis() - startTime >= timeoutMs) {
             return API_RECEIVE_ERROR_TIMEOUT_DATA;
         }
         self->htable->PortDelay(1);  // Add a 1 ms delay to prevent busy-waiting
     }
 
     return API_RECEIVE_SUCCESS;
 }
 
 /**
  * @brief Checks for and receives an XBee API frame, populating the provided frame pointer.
  * 
  * This function attempts to read and receive an XBee API frame from the UART interface. 
  * It validates the received data by checking the start delimiter, frame length, and checksum. 
  * If the frame is successfully received and validated, the frame structure is populated 
  * with the received data. The function returns API_RECEIVE_SUCCESS if successful, or an 
  * error code from `api_receive_status_t` if any step in the process fails, including timeout.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[out] frame Pointer to an `xbee_api_frame_t` structure where the received frame data will be stored.
  * 
  * @return api_receive_status_t Returns API_RECEIVE_SUCCESS if the frame is successfully received, or an error code if a failure occurs.
  */
 api_receive_status_t apiReceiveApiFrame(XBee* self, xbee_api_frame_t *frame) {
     if (!frame) {
         APIFrameDebugPrint("Error: Invalid frame pointer. The frame pointer passed to the function is NULL.\n");
         return API_RECEIVE_ERROR_INVALID_POINTER;
     }
 
     memset(frame, 0, sizeof(xbee_api_frame_t));
 
     // Attempt to read the start delimiter with timeout
     uint8_t start_delimiter;
     api_receive_status_t result = readBytesWithTimeout(self, &start_delimiter, 1, UART_READ_TIMEOUT_MS);
     if (result != API_RECEIVE_SUCCESS) {
         //APIFrameDebugPrint("Error: Timeout occurred while waiting to read start delimiter.\n");
         return API_RECEIVE_ERROR_TIMEOUT_START_DELIMITER;
     }
     APIFrameDebugPrint("Start delimiter received: 0x%02X\n", start_delimiter);
 
     if (start_delimiter != 0x7E) {
         APIFrameDebugPrint("Error: Invalid start delimiter. Expected 0x7E, but received 0x%02X.\n", start_delimiter);
         return API_RECEIVE_ERROR_INVALID_START_DELIMITER;
     }
 
     // Read length with timeout
     uint8_t length_bytes[2];
     result = readBytesWithTimeout(self, length_bytes, 2, UART_READ_TIMEOUT_MS);
     if (result != API_RECEIVE_SUCCESS) {
         APIFrameDebugPrint("Error: Timeout occurred while waiting to read frame length.\n");
         return API_RECEIVE_ERROR_TIMEOUT_LENGTH;
     }
     uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
     APIFrameDebugPrint("Frame length received: %d bytes\n", length);
 
     //@todo: Dynamic size based on api frame length
     if (length > XBEE_MAX_FRAME_DATA_SIZE) {
         APIFrameDebugPrint("Error: Frame length exceeds buffer size.\n");
         return API_RECEIVE_ERROR_FRAME_TOO_LARGE;
     }
 
     // Read the frame data with timeout
     result = readBytesWithTimeout(self, frame->data, length, UART_READ_TIMEOUT_MS);
     if (result != API_RECEIVE_SUCCESS) {
         APIFrameDebugPrint("Error: Timeout occurred while waiting to read frame data.\n");
         return API_RECEIVE_ERROR_TIMEOUT_DATA;
     }
     APIFrameDebugPrint("Complete frame data received: ");
     for (int i = 0; i < length; i++) {
         APIFrameDebugPrint("0x%02X ", frame->data[i]);
     }
     APIFrameDebugPrint("\n");
 
     // Read the checksum with timeout
     result = readBytesWithTimeout(self, &(frame->checksum), 1, UART_READ_TIMEOUT_MS);
     if (result != API_RECEIVE_SUCCESS) {
         APIFrameDebugPrint("Error: Timeout occurred while waiting to read checksum.\n");
         return API_RECEIVE_ERROR_TIMEOUT_CHECKSUM;
     }
 
     // Populate frame structure
     frame->length = length;
     frame->type = frame->data[0];
 
     // Check and verify the checksum
     uint8_t checksum = frame->checksum;
     for (int i = 0; i < length; i++) {
         checksum += frame->data[i];
     }
     if (checksum != 0xFF) {
         APIFrameDebugPrint("Error: Invalid checksum. Expected 0xFF, but calculated 0x%02X.\n", checksum);
         return API_RECEIVE_ERROR_INVALID_CHECKSUM;
     }
 
     return API_RECEIVE_SUCCESS; // Successfully received a frame
 }
 
 
 /**
  * @brief Calls registered handlers based on the received API frame type.
  * 
  * This function processes a received XBee API frame by calling the appropriate 
  * handler function based on the frame's type. It supports handling AT responses, 
  * modem status, transmit status, and received packet frames. For each frame type, 
  * the corresponding handler function is invoked if it is registered in the XBee 
  * virtual table (vtable). If the frame type is unknown, a debug message is printed.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] frame The received API frame to be handled.
  * 
  * @return void This function does not return a value.
  */
 void apiHandleFrame(XBee* self, xbee_api_frame_t frame){
     switch (frame.type) {
         case XBEE_API_TYPE_AT_RESPONSE:
             xbeeHandleAtResponse(self, &frame);
             break;
         case XBEE_API_TYPE_MODEM_STATUS:
             xbeeHandleModemStatus(self, &frame);
             break;
         case XBEE_API_TYPE_TX_STATUS:
         case XBEE_API_TYPE_LR_EXPLICIT_TX_STATUS:
             if(self->vtable->handleTransmitStatusFrame){
                 self->vtable->handleTransmitStatusFrame(self, &frame);
             }
             break;
         case XBEE_API_TYPE_LR_RX_PACKET:
         case XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET:
         case XBEE_API_TYPE_CELLULAR_SOCKET_RX:
         case XBEE_API_TYPE_CELLULAR_SOCKET_RX_FROM:
             if(self->vtable->handleRxPacketFrame){
                 self->vtable->handleRxPacketFrame(self, &frame);
             }
             break;
         default:
             APIFrameDebugPrint("Received unknown frame type: 0x%02X\n", frame.type);
             break;
     }
 }
 
 /**
  * @brief Sends an AT command via an API frame and waits for the response.
  * 
  * This function sends an AT command using an XBee API frame and then waits for a response 
  * from the XBee module within a specified timeout period. The response is captured and 
  * stored in the provided response buffer. The function continuously checks for incoming 
  * frames and processes them until the expected AT response frame is received or the 
  * timeout period elapses.
  * 
  * @param[in] self Pointer to the XBee instance.
  * @param[in] command The AT command to be sent, specified as an `at_command_t` enum.
  * @param[in] parameter Pointer to the parameter data to be included with the AT command (can be NULL).
  * @param[out] responseBuffer Pointer to a buffer where the AT command response will be stored.
  * @param[out] responseLength Pointer to a variable where the length of the response will be stored.
  * @param[in] timeoutMs The timeout period in milliseconds within which the response must be received.
  * @param[in] responseBufferSize The maximum size of the response buffer.
  * 
  * @return int Returns 0 (`API_SEND_SUCCESS`) if the AT command is successfully sent and a valid response is received, 
  * or a non-zero error code if there is a failure (`API_SEND_AT_CMD_ERROR`, `API_SEND_AT_CMD_RESPONSE_TIMEOUT`, etc.).
  */
 int apiSendAtCommandAndGetResponse(XBee* self, at_command_t command, const uint8_t *parameter, uint8_t paramLength, uint8_t *responseBuffer, 
     uint8_t *responseLength, uint32_t timeoutMs, uint16_t responseBufferSize) {
     // Send the AT command using API frame
     apiSendAtCommand(self, command, (const uint8_t *)parameter, paramLength);
 
     // Get the start time using the platform-specific function
     uint32_t startTime = self->htable->PortMillis();
     
     // Wait and receive the response within the timeout period
     xbee_api_frame_t frame;
     int status;

    const char* cmdStr = atCommandToString(command);
     if (!cmdStr || strlen(cmdStr) != 2) {
         APIFrameDebugPrint("Invalid AT command enum.\n");
         return API_SEND_ERROR_INVALID_COMMAND;
     }
 
     while (1) {
         // Attempt to receive the API frame
         status = apiReceiveApiFrame(self, &frame);
 
         // Check if a valid frame was received
         if (status == 0) {
             // Check if the received frame is an AT response
             if (frame.type == XBEE_API_TYPE_AT_RESPONSE) {
                
                // Verify response matches the requested AT command
                 if (frame.data[2] != cmdStr[0] || frame.data[3] != cmdStr[1]) {
                     APIFrameDebugPrint("Mismatched AT command response: expected %s, got %c%c\n",
                         cmdStr, frame.data[2], frame.data[3]);
                     continue; // Keep waiting
                 }

                 // Extract the AT command response
                 *responseLength = frame.length - 5;  // Subtract the frame ID and AT command bytes
                 APIFrameDebugPrint("responseLength: %u\n", *responseLength);
                 if(frame.data[4] == 0){
                    if (*responseLength > responseBufferSize) {
                        APIFrameDebugPrint("Response exceeds buffer size: %u > %u\n", *responseLength, responseBufferSize);
                        return API_SEND_AT_CMD_ERROR;
                    }

                     if((responseBuffer != NULL) && (*responseLength)){
                         memcpy(responseBuffer, &frame.data[5], *responseLength);
                     }
                 }else{
                     APIFrameDebugPrint("API Frame AT CMD Error.\n");
                     return API_SEND_AT_CMD_ERROR;
                 }
             
                 // Return success
                 return API_SEND_SUCCESS;
             } 
             else{
                 apiHandleFrame(self, frame);
             }
         }
 
         // Check if the timeout period has elapsed using platform-specific time
         if ((self->htable->PortMillis() - startTime) >= timeoutMs) {
             APIFrameDebugPrint("Timeout waiting for AT response.\n");
             return API_SEND_AT_CMD_RESONSE_TIMEOUT;
         }
         
         self->htable->PortDelay(1);
     }
 }
 
 //Print out AT Response
 void xbeeHandleAtResponse(XBee* self, xbee_api_frame_t *frame) {
    (void)self;
 #if API_FRAME_DEBUG_PRINT_ENABLED
     // The first byte of frame->data is the Frame ID
     uint8_t frame_id = frame->data[1];
 
     // The next two bytes are the AT command
     char at_command[3];
     at_command[0] = frame->data[2];
     at_command[1] = frame->data[3];
     at_command[2] = '\0'; // Null-terminate the string
 
 
     // The next byte is the command status
     uint8_t command_status = frame->data[4];
 #endif
 
     // Print the basic information
     APIFrameDebugPrint("AT Response:\n");
     APIFrameDebugPrint("  Frame ID: %d\n", frame_id);
     APIFrameDebugPrint("  AT Command: %s\n", at_command);
     APIFrameDebugPrint("  Command Status: %d\n", command_status);
 
     // Check if there is additional data in the frame
     if (frame->length > 5) {
         APIFrameDebugPrint("  Data: ");
         // Print the remaining data bytes
         APIFrameDebugPrint("%s\n", &(frame->data[5]));
     } else {
         APIFrameDebugPrint("  No additional data.\n");
     }
 }
 
 //Should be moved to be handled by user?
 void xbeeHandleModemStatus(XBee* self, xbee_api_frame_t *frame) {
     (void)self;
     if (frame->type != XBEE_API_TYPE_MODEM_STATUS) return;
 
     APIFrameDebugPrint("Modem Status: %d\n", frame->data[1]);
     // Add further processing as needed
 }
 
 /**
  * @brief Converts an ASCII string to a hex array.
  * 
  * This function takes an ASCII string and converts it into a hexadecimal
  * representation where each character pair forms a hex byte.
  * 
  * @param[in] asciiStr Pointer to the ASCII string.
  * @param[out] hexArray Pointer to the output hex array.
  * @param[in] maxLen Maximum allowed length of the hex array.
  * 
  * @return int Number of bytes written to hex_array, or -1 if an error occurs.
  */
 int asciiToHexArray(const char *asciiStr, uint8_t *hexArray, size_t maxLen) {
     if (!asciiStr || !hexArray) {
         return -1; // Error: Null pointer
     }
 
     size_t asciiLen = strlen(asciiStr);
     if (asciiLen % 2 != 0 || asciiLen / 2 > maxLen) {
         return -1; // Error: Invalid length
     }
 
     for (size_t i = 0; i < asciiLen; i += 2) {
         char hexByte[3] = {asciiStr[i], asciiStr[i + 1], '\0'};
         hexArray[i / 2] = (uint8_t)strtol(hexByte, NULL, 16);
     }
 
     return asciiLen / 2;
 }

