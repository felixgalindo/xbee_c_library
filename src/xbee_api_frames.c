/**
 * @file xbee_api_frames.c
 * @brief Implementation of XBee API frame handling.
 * 
 * This file contains the implementation of functions used to create, parse, 
 * and handle API frames for XBee communication. API frames are the primary 
 * method for structured data exchange with XBee modules.
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
static uint8_t calculate_checksum(const uint8_t *frame, uint16_t len) {
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
 * @param[in] frame_type The type of the API frame to send.
 * @param[in] data Pointer to the frame data to be included in the API frame.
 * @param[in] len Length of the frame data in bytes.
 * 
 * @return int Returns 0 (`API_SEND_SUCCESS`) if the frame is successfully sent, 
 * or a non-zero error code (`API_SEND_ERROR_UART_FAILURE`) if there is a failure.
 */
int api_send_frame(XBee* self,uint8_t frame_type, const uint8_t *data, uint16_t len) {
    uint8_t frame[256];
    uint16_t frame_length = 0;
    self->frameIdCntr++;
    if(self->frameIdCntr == 0) self->frameIdCntr = 1; //reset frame counter when 0

    // Start delimiter
    frame[frame_length++] = 0x7E;

    // Length MSB and LSB
    frame[frame_length++] = (len + 1) >> 8;
    frame[frame_length++] = (len + 1) & 0xFF;

    // Frame type
    frame[frame_length++] = frame_type;

    // Frame data
    memcpy(&frame[frame_length], data, len);
    frame_length += len;

    // Calculate and add checksum
    frame[frame_length] = calculate_checksum(frame, frame_length);
    frame_length++;

    // Print the API frame in hex format
    API_FRAME_DEBUG_PRINT("Sending API Frame: ");
    for (uint16_t i = 0; i < frame_length; i++) {
        API_FRAME_DEBUG_PRINT("0x%02X ", frame[i]);
    }
    API_FRAME_DEBUG_PRINT("\n");

    // Send frame via UART
    int uart_status = self->htable->PortUartWrite(frame, frame_length);
    if (uart_status != 0) {
        return API_SEND_ERROR_UART_FAILURE;
    }

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
 * @param[in] param_length Length of the parameter data in bytes (0 if no parameters).
 * 
 * @return int Returns 0 (`API_SEND_SUCCESS`) if the AT command is successfully sent, 
 * or a non-zero error code if there is a failure (`API_SEND_ERROR_FRAME_TOO_LARGE`, 
 * `API_SEND_ERROR_INVALID_COMMAND`, etc.).
 */
int api_send_at_command(XBee* self,at_command_t command, const uint8_t *parameter, uint8_t param_length) {
    uint8_t frame_data[128];
    uint16_t frame_length = 0;

   // Check if the parameter length is too large
    if (param_length > 128) {
        return API_SEND_ERROR_FRAME_TOO_LARGE;
    }

    // Frame ID
    frame_data[frame_length++] = self->frameIdCntr;

    // AT Command (2 bytes)
    const char *cmd_str = at_command_to_string(command);
    frame_data[frame_length++] = cmd_str[0];
    frame_data[frame_length++] = cmd_str[1];

    if (cmd_str == NULL) {
        return API_SEND_ERROR_INVALID_COMMAND;
    }

    // AT Command Parameter
    if (param_length > 0) {
        memcpy(&frame_data[frame_length], parameter, param_length);
        frame_length += param_length;
    }

    // Print the AT command and parameter in a readable format
    API_FRAME_DEBUG_PRINT("Sending AT Command: %s\n", cmd_str);
    if (param_length > 0) {
        API_FRAME_DEBUG_PRINT("Parameter: ");
        for (uint8_t i = 0; i < param_length; i++) {
            API_FRAME_DEBUG_PRINT("0x%02X ", parameter[i]);
        }
        API_FRAME_DEBUG_PRINT("\n");
    } else {
        API_FRAME_DEBUG_PRINT("No Parameters\n");
    }

    // Use api_send_frame to send the complete frame
    return api_send_frame(self, XBEE_API_TYPE_AT_COMMAND, frame_data, frame_length);
}

/**
 * @brief Checks for and receives an XBee API frame, populating the provided frame pointer.
 * 
 * This function attempts to read and receive an XBee API frame from the UART interface. 
 * It validates the received data by checking the start delimiter, frame length, and checksum. 
 * If the frame is successfully received and validated, the frame structure is populated 
 * with the received data. The function returns 0 if successful, or a negative error code 
 * if any step in the process fails.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[out] frame Pointer to an `xbee_api_frame_t` structure where the received frame data will be stored.
 * 
 * @return int Returns 0 if the frame is successfully received, or a negative error code indicating the type of failure.
 */
int api_receive_api_frame(XBee* self,xbee_api_frame_t *frame) {
    if (!frame) {
        API_FRAME_DEBUG_PRINT("Error: Invalid frame pointer. The frame pointer passed to the function is NULL.\n");
        return -1;
    }

    memset(frame, 0, sizeof(xbee_api_frame_t));

    // Attempt to read the start delimiter
    uint8_t start_delimiter;
    int bytes_received;
    uart_status_t status = self->htable->PortUartRead(&start_delimiter, 1, &bytes_received);

    if (status != UART_SUCCESS || bytes_received <= 0) {
        if (status == UART_ERROR_TIMEOUT) {
            API_FRAME_DEBUG_PRINT("Error: Timeout occurred while waiting to read start delimiter. No data received within %d ms.\n", UART_READ_TIMEOUT_MS);
        } else {
            API_FRAME_DEBUG_PRINT("Error: Failed to read start delimiter. Status: %d, Bytes received: %d\n", status, bytes_received);
        }
        return -2;
    }
    API_FRAME_DEBUG_PRINT("Start delimiter received: 0x%02X\n", start_delimiter);

    if (start_delimiter != 0x7E) {
        API_FRAME_DEBUG_PRINT("Error: Invalid start delimiter. Expected 0x7E, but received 0x%02X.\n", start_delimiter);
        return -3;
    }

    // Read length
    uint8_t length_bytes[2];
    status = self->htable->PortUartRead(length_bytes, 2, &bytes_received);

    if (status != UART_SUCCESS || bytes_received != 2) {
        if (status == UART_ERROR_TIMEOUT) {
            API_FRAME_DEBUG_PRINT("Error: Timeout occurred while waiting to read frame length. No data received within %d ms.\n", UART_READ_TIMEOUT_MS);
        } else {
            API_FRAME_DEBUG_PRINT("Error: Failed to read frame length. Status: %d, Bytes received: %d\n", status, bytes_received);
        }
        return -4;
    }

    uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
    API_FRAME_DEBUG_PRINT("Frame length received: %d bytes\n", length);

    if (length > XBEE_MAX_FRAME_DATA_SIZE) {
        API_FRAME_DEBUG_PRINT("Error: Frame length exceeds buffer size. Received length: %d bytes, but maximum allowed is %d bytes.\n", length, XBEE_MAX_FRAME_DATA_SIZE);
        return -5;
    }

    // Read the frame data
    status = self->htable->PortUartRead(frame->data, length, &bytes_received);

    if (status != UART_SUCCESS || bytes_received != length) {
        if (status == UART_ERROR_TIMEOUT) {
            API_FRAME_DEBUG_PRINT("Error: Timeout occurred while waiting to read frame data. Expected %d bytes, but received %d bytes within %d ms.\n", length, bytes_received, UART_READ_TIMEOUT_MS);
        } else {
            API_FRAME_DEBUG_PRINT("Error: Failed to read complete frame data. Status: %d, Expected: %d bytes, Received: %d bytes.\n", status, length, bytes_received);
        }
        return -6;
    }

    // Print the received frame data
    API_FRAME_DEBUG_PRINT("Complete frame data received: ");
    for (int i = 0; i < bytes_received; i++) {
        API_FRAME_DEBUG_PRINT("0x%02X ", frame->data[i]);
    }
    API_FRAME_DEBUG_PRINT("\n");

    // Read the checksum
    status = self->htable->PortUartRead(&(frame->checksum), 1, &bytes_received);

    if (status != UART_SUCCESS || bytes_received != 1) {
        if (status == UART_ERROR_TIMEOUT) {
            API_FRAME_DEBUG_PRINT("Error: Timeout occurred while waiting to read checksum. Expected 1 byte, but received %d bytes within %d ms.\n", bytes_received, UART_READ_TIMEOUT_MS);
        } else {
            API_FRAME_DEBUG_PRINT("Error: Failed to read complete frame data. Status: %d, Expected: 1 bytes, Received: %d bytes.\n", status,bytes_received);
        }
        return -7;
    }

    // Populate frame structure
    frame->length = length;
    frame->type = frame->data[0];

    // Check and verify the checksum
    uint8_t checksum = frame->checksum;
    for (int i = 0; i < (length); i++) {
        checksum += frame->data[i];
    }
    if (checksum != 0xFF) {
        API_FRAME_DEBUG_PRINT("Error: Invalid checksum. Expected 0xFF, but calculated 0x%02X.\n", checksum);
        return -8;
    }

    return 0; // Successfully received a frame
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
void api_handle_frame(XBee* self, xbee_api_frame_t frame){
    switch (frame.type) {
        case XBEE_API_TYPE_AT_RESPONSE:
            xbee_handle_at_response(self, &frame);
            break;
        case XBEE_API_TYPE_MODEM_STATUS:
            xbee_handle_modem_status(self, &frame);
            break;
        case XBEE_API_TYPE_TX_STATUS:
            if(self->vtable->handle_transmit_status_frame){
                self->vtable->handle_transmit_status_frame(self, &frame);
            }
            break;
        case XBEE_API_TYPE_LR_RX_PACKET:
        case XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET:
            if(self->vtable->handle_rx_packet_frame){
                self->vtable->handle_rx_packet_frame(self, &frame);
            }
            break;
        default:
            API_FRAME_DEBUG_PRINT("Received unknown frame type: 0x%02X\n", frame.type);
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
 * @param[out] response_buffer Pointer to a buffer where the AT command response will be stored.
 * @param[out] response_length Pointer to a variable where the length of the response will be stored.
 * @param[in] timeout_ms The timeout period in milliseconds within which the response must be received.
 * 
 * @return int Returns 0 (`API_SEND_SUCCESS`) if the AT command is successfully sent and a valid response is received, 
 * or a non-zero error code if there is a failure (`API_SEND_AT_CMD_ERROR`, `API_SEND_AT_CMD_RESPONSE_TIMEOUT`, etc.).
 */
int api_send_at_command_and_get_response(XBee* self, at_command_t command, const char *parameter, uint8_t *response_buffer, 
    uint8_t *response_length, uint32_t timeout_ms) {
    // Send the AT command using API frame
    uint8_t param_length = (parameter != NULL) ? strlen(parameter) : 0;
    api_send_at_command(self, command, (const uint8_t *)parameter, param_length);

    // Get the start time using the platform-specific function
    uint32_t start_time = self->htable->PortMillis();
    
    // Wait and receive the response within the timeout period
    xbee_api_frame_t frame;
    int status;

    while (1) {
        // Attempt to receive the API frame
        status = api_receive_api_frame(self, &frame);

        // Check if a valid frame was received
        if (status == 0) {
            // Check if the received frame is an AT response
            if (frame.type == XBEE_API_TYPE_AT_RESPONSE) {

                // Extract the AT command response
                *response_length = frame.length - 5;  // Subtract the frame ID and AT command bytes
                API_FRAME_DEBUG_PRINT("response_length: %u\n", *response_length);
                if(frame.data[4] == 0){
                    if((response_buffer != NULL) && (*response_length)){
                        memcpy(response_buffer, &frame.data[5], *response_length);
                    }
                }else{
                    API_FRAME_DEBUG_PRINT("API Frame AT CMD Error.\n");
                    return API_SEND_AT_CMD_ERROR;
                }
            
                // Return success
                return API_SEND_SUCCESS;
            } 
            else{
                api_handle_frame(self, frame);
            }
        }

        // Check if the timeout period has elapsed using platform-specific time
        if ((self->htable->PortMillis() - start_time) >= timeout_ms) {
            API_FRAME_DEBUG_PRINT("Timeout waiting for AT response.\n");
            return API_SEND_AT_CMD_RESONSE_TIMEOUT;
        }
        
        self->htable->PortDelay(1);
    }
}

//Print out AT Response
void xbee_handle_at_response(XBee* self, xbee_api_frame_t *frame) {
    // The first byte of frame->data is the Frame ID
    uint8_t frame_id = frame->data[1];

    // The next two bytes are the AT command
    char at_command[3];
    at_command[0] = frame->data[2];
    at_command[1] = frame->data[3];
    at_command[2] = '\0'; // Null-terminate the string

    // The next byte is the command status
    uint8_t command_status = frame->data[4];

    // Print the basic information
    API_FRAME_DEBUG_PRINT("AT Response:\n");
    API_FRAME_DEBUG_PRINT("  Frame ID: %d\n", frame_id);
    API_FRAME_DEBUG_PRINT("  AT Command: %s\n", at_command);
    API_FRAME_DEBUG_PRINT("  Command Status: %d\n", command_status);

    // Check if there is additional data in the frame
    if (frame->length > 5) {
        API_FRAME_DEBUG_PRINT("  Data: ");
        // Print the remaining data bytes
        API_FRAME_DEBUG_PRINT("%s\n", &(frame->data[5]));
    } else {
        API_FRAME_DEBUG_PRINT("  No additional data.\n");
    }
}

//Should be moved to be handled by user?
void xbee_handle_modem_status(XBee* self, xbee_api_frame_t *frame) {
    if (frame->type != XBEE_API_TYPE_MODEM_STATUS) return;

    API_FRAME_DEBUG_PRINT("Modem Status: %d\n", frame->data[1]);
    // Add further processing as needed
}
