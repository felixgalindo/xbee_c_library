/**
 * @file xbee_api_frames.h
 * @brief Header file for XBee API frame handling.
 * 
 * This file defines the interface for creating, parsing, and handling XBee API frames.
 * API frames are the primary method for structured data exchange with XBee modules,
 * enabling the control and configuration of the modules as well as data transmission.
 * This file includes prototypes for functions that manage the construction, validation,
 * and interpretation of these frames.
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

 #ifndef XBEE_FRAMES_H
 #define XBEE_FRAMES_H
 
 #if defined(__cplusplus)
 extern "C"
 {
 #endif
 
 #include "xbee_at_cmds.h"
 #include "xbee.h"
 #include "config.h"
 
 #define XBEE_MAX_FRAME_DATA_SIZE 1024 //@todo: Dynamic size based on api frame length
 #define API_SEND_SUCCESS 0
 #define API_SEND_ERROR_TIMEOUT -1
 #define API_SEND_ERROR_INVALID_COMMAND -2
 #define API_SEND_ERROR_UART_FAILURE -3
 #define API_SEND_ERROR_FRAME_TOO_LARGE -4
 #define API_SEND_AT_CMD_ERROR -5
 #define API_SEND_AT_CMD_RESONSE_TIMEOUT -6
 #define API_RECEIVE_ERROR_NULL_FRAME -7              
 #define API_SEND_ERROR_BUFFER_TOO_SMALL -8           
 
 /**
  * @enum xbee_deliveryStatus_t
  * @brief Delivery status codes for XBee Transmit Status frames (0x89).
  *
  * This enumeration defines the possible delivery status codes that can be 
  * returned in the Transmit Status frame (0x89) by the XBee module. These codes 
  * indicate the result of a data transmission, helping to determine whether the 
  * transmission was successful or if an error occurred.
  *
  * The status codes can be used to troubleshoot issues in communication by 
  * identifying the specific reason for a transmission failure.
  *
  * @var XBEE_DELIVERY_STATUS_SUCCESS
  *     Indicates that the transmission was successful. The packet was delivered to 
  *     the intended recipient without any errors.
  * @var XBEE_DELIVERY_STATUS_NO_ACK
  *     Indicates that no acknowledgment was received from the recipient. This 
  *     typically means that the packet was not successfully received or acknowledged.
  * @var XBEE_DELIVERY_STATUS_CCA_FAILURE
  *     Indicates a Clear Channel Assessment (CCA) failure. The XBee module was 
  *     unable to transmit because the channel was not clear (e.g., due to interference).
  * @var XBEE_DELIVERY_STATUS_PURGED
  *     Indicates that the transmission was purged from the queue and not sent. This 
  *     may occur if the module was unable to transmit the packet within the allowed 
  *     timeframe.
  * @var XBEE_DELIVERY_STATUS_INVALID_DEST
  *     Indicates that the destination endpoint was invalid. The packet could not be 
  *     delivered because the specified destination was not valid.
  * @var XBEE_DELIVERY_STATUS_NET_ACK_FAILURE
  *     Indicates a network acknowledgment failure. The packet was not acknowledged 
  *     at the network level, suggesting that it was not delivered successfully.
  * @var XBEE_DELIVERY_STATUS_NOT_JOINED
  *     Indicates that the module is not joined to a network. Transmission failed 
  *     because the XBee module is not part of a network.
  * @var XBEE_DELIVERY_STATUS_SELF_ADDRESSED
  *     Indicates that the transmission was attempted to the module's own address, 
  *     which is not allowed.
  * @var XBEE_DELIVERY_STATUS_ADDRESS_NOT_FOUND
  *     Indicates that the destination address was not found. The XBee module could 
  *     not find the specified address in the network.
  * @var XBEE_DELIVERY_STATUS_ROUTE_NOT_FOUND
  *     Indicates that no route to the destination was found. The module could not 
  *     find a valid path to the destination address within the network.
  * @var XBEE_DELIVERY_STATUS_PAYLOAD_TOO_LARGE
  *     Indicates that the payload was too large for transmission. The size of the 
  *     data exceeded the maximum allowed payload size for the XBee module.
  */
 typedef enum {
     XBEE_DELIVERY_STATUS_SUCCESS = 0x00,         // Success
     XBEE_DELIVERY_STATUS_NO_ACK = 0x01,          // No acknowledgment received
     XBEE_DELIVERY_STATUS_CCA_FAILURE = 0x02,     // Clear Channel Assessment (CCA) failure
     XBEE_DELIVERY_STATUS_PURGED = 0x03,          // Transmission purged
     XBEE_DELIVERY_STATUS_INVALID_DEST = 0x15,    // Invalid destination endpoint
     XBEE_DELIVERY_STATUS_NET_ACK_FAILURE = 0x21, // Network acknowledgment failure
     XBEE_DELIVERY_STATUS_NOT_JOINED = 0x22,      // Not joined to network
     XBEE_DELIVERY_STATUS_SELF_ADDRESSED = 0x23,  // Attempted to transmit to self
     XBEE_DELIVERY_STATUS_ADDRESS_NOT_FOUND = 0x24, // Address not found
     XBEE_DELIVERY_STATUS_ROUTE_NOT_FOUND = 0x25, // Route not found
     XBEE_DELIVERY_STATUS_PAYLOAD_TOO_LARGE = 0x74 // Payload too large
 } xbee_deliveryStatus_t;
 
 /**
  * @enum xbee_api_frame_type_t
  * @brief Enumeration of XBee API frame types.
  *
  * This enumeration defines the various API frame types used in XBee communication.
  * API frames are structured data packets that facilitate communication between the
  * host (e.g., microcontroller) and the XBee module. Different frame types serve
  * different purposes, such as sending commands, receiving data, controlling GPIOs,
  * or reading ADC values.
  *
  * The frame types are divided into common frames, specific frames for XBee LR,
  * XBee 3 RF, XBee Cellular, and GPIO/ADC-related frames.
  *
  * @var XBEE_API_TYPE_AT_COMMAND
  *     Frame type for sending AT commands to the XBee module. The command is executed
  *     locally by the module.
  * @var XBEE_API_TYPE_TX_REQUEST
  *     Frame type for transmitting data to another device in the network. This frame
  *     initiates the transmission of a data packet.
  * @var XBEE_API_TYPE_MODEM_STATUS
  *     Frame type that reports the status of the modem (e.g., hardware reset, network join).
  *     This frame is automatically generated by the XBee module.
  * @var XBEE_API_TYPE_AT_RESPONSE
  *     Frame type for receiving the response to a previously sent AT command. It contains
  *     the result of the command execution.
  * @var XBEE_API_TYPE_TX_STATUS
  *     Frame type that reports the delivery status of a previously sent TX Request. This
  *     frame provides feedback on whether the transmission was successful or failed.
  *
  * @var XBEE_API_TYPE_LR_JOIN_REQUEST
  *     Frame type specific to XBee LR modules for sending a Join Request in a LoRaWAN network.
  *     This frame is used to join the network.
  * @var XBEE_API_TYPE_LR_TX_REQUEST
  *     Frame type specific to XBee LR modules for transmitting data in a LoRaWAN network. It
  *     initiates the transmission of a data packet within the LoRaWAN.
  * @var XBEE_API_TYPE_LR_RX_PACKET
  *     Frame type specific to XBee LR modules for receiving data packets from the LoRaWAN network.
  *     This frame carries the payload received from another device.
  * @var XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET
  *     Frame type specific to XBee LR modules for receiving explicitly addressed data packets
  *     from the LoRaWAN network. It provides additional addressing information with the payload.
  *
  * @var XBEE_API_TYPE_3RF_REMOTE_AT_COMMAND
  *     Frame type specific to XBee 3 RF modules for sending remote AT commands to another
  *     device in the network. This allows configuration of remote devices.
  * @var XBEE_API_TYPE_3RF_REMOTE_AT_RESPONSE
  *     Frame type specific to XBee 3 RF modules for receiving responses to remote AT commands.
  *     It contains the result of the remote command execution.
  * @var XBEE_API_TYPE_3RF_RX_PACKET
  *     Frame type specific to XBee 3 RF modules for receiving data packets from another device
  *     in the network. This frame carries the received payload.
  * @var XBEE_API_TYPE_3RF_RX_EXPLICIT_PACKET
  *     Frame type specific to XBee 3 RF modules for receiving explicitly addressed data packets.
  *     It includes addressing information along with the received payload.
  *
  * @var XBEE_API_TYPE_CELLULAR_TX_IPV4
  *     Frame type specific to XBee Cellular modules for transmitting IPv4 data packets. It initiates
  *     the transmission of an IPv4 packet over the cellular network.
  * @var XBEE_API_TYPE_CELLULAR_RX_IPV4
  *     Frame type specific to XBee Cellular modules for receiving IPv4 data packets. It carries
  *     the received IPv4 packet from the cellular network.
  * @var XBEE_API_TYPE_CELLULAR_MODEM_STATUS
  *     Frame type specific to XBee Cellular modules that reports the status of the cellular modem,
  *     including network registration, signal strength, and other modem status information.
  *
  * @var XBEE_API_TYPE_IO_DATA_SAMPLE_RX
  *     Frame type for receiving IO data samples. This frame contains the values read from the 
  *     GPIOs and ADCs of the remote XBee module.
  * @var XBEE_API_TYPE_REMOTE_AT_COMMAND
  *     Frame type for sending remote AT commands related to GPIO/ADC configuration or control. 
  *     This allows configuration of remote XBee modules.
  * @var XBEE_API_TYPE_REMOTE_AT_RESPONSE
  *     Frame type for receiving responses to remote AT commands related to GPIO/ADC configuration
  *     or control. It contains the result of the command execution.
  * @var XBEE_API_TYPE_IO_SAMPLE_RX_INDICATOR
  *     Frame type for receiving IO samples with explicit addressing. This frame includes
  *     GPIO and ADC readings from a specific XBee module and additional addressing information.
  *
  * Additional frame types can be added to this enumeration as needed to support new functionality
  * or XBee modules.
  */
 typedef enum {
 
     /**< XBee Common API Frames */
     XBEE_API_TYPE_AT_COMMAND = 0x08,               ///< Frame for sending AT commands
     XBEE_API_TYPE_TX_REQUEST = 0x10,               ///< Frame for transmitting data
     XBEE_API_TYPE_MODEM_STATUS = 0x8A,             ///< Frame for modem status reports
     XBEE_API_TYPE_AT_RESPONSE = 0x88,              ///< Frame for receiving AT command responses
     XBEE_API_TYPE_TX_STATUS = 0x89,                ///< Frame for delivery status reports
    
     //@todo: Move XBee specific API frames to their respective modules
     
     /**< XBee LR Specific API Frames */
     XBEE_API_TYPE_LR_JOIN_REQUEST = 0x14,          ///< Frame for LoRaWAN join requests
     XBEE_API_TYPE_LR_TX_REQUEST = 0x50,            ///< Frame for transmitting data in LoRaWAN
     XBEE_API_TYPE_LR_RX_PACKET = 0xD0,             ///< Frame for receiving data packets in LoRaWAN
     XBEE_API_TYPE_LR_EXPLICIT_RX_PACKET = 0xD1,    ///< Frame for receiving explicitly addressed packets in LoRaWAN
     XBEE_API_TYPE_LR_EXPLICIT_TX_STATUS = 0xD2,    ///< Frame for explicit TX status in LoRaWAN
 
     /**< XBee 3 RF Specific API Frames */
     XBEE_API_TYPE_3RF_REMOTE_AT_COMMAND = 0x17,    ///< Frame for sending remote AT commands (XBee 3 RF)
     XBEE_API_TYPE_3RF_REMOTE_AT_RESPONSE = 0x97,   ///< Frame for receiving remote AT responses (XBee 3 RF)
     XBEE_API_TYPE_3RF_RX_PACKET = 0x90,            ///< Frame for receiving data packets (XBee 3 RF)
     XBEE_API_TYPE_3RF_RX_EXPLICIT_PACKET = 0x91,   ///< Frame for receiving explicitly addressed packets (XBee 3 RF)
 
     /**< XBee Cellular Specific API Frames */
     XBEE_API_TYPE_CELLULAR_TX_IPV4 = 0x20,         ///< Frame for transmitting IPv4 data (XBee Cellular)
     XBEE_API_TYPE_CELLULAR_RX_IPV4 = 0xB0,         ///< Frame for receiving IPv4 data (XBee Cellular)
     XBEE_API_TYPE_CELLULAR_MODEM_STATUS = 0x8A,    ///< Frame for cellular modem status (XBee Cellular)
     XBEE_API_TYPE_CELLULAR_SOCKET_CREATE = 0x40,   ///< Frame to create a socket
     XBEE_API_TYPE_CELLULAR_SOCKET_OPTION = 0x41,   ///< Frame to set socket options
     XBEE_API_TYPE_CELLULAR_SOCKET_CONNECT = 0x42,  ///< Frame to connect a socket
     XBEE_API_TYPE_CELLULAR_SOCKET_CLOSE = 0x43,    ///< Frame to close a socket
     XBEE_API_TYPE_CELLULAR_SOCKET_SEND = 0x44,     ///< Frame to send socket data
     XBEE_API_TYPE_CELLULAR_SOCKET_SEND_TO = 0x45,  ///< Frame to send UDP data to a specific address
     XBEE_API_TYPE_CELLULAR_SOCKET_BIND = 0x46,     ///< Frame to bind a socket to a local port
     XBEE_API_TYPE_CELLULAR_SOCKET_CREATE_RESPONSE = 0xC0, ///< Response to socket create
     XBEE_API_TYPE_CELLULAR_SOCKET_OPTION_RESPONSE = 0xC1, ///< Response to socket option
     XBEE_API_TYPE_CELLULAR_SOCKET_CONNECT_RESPONSE = 0xC2, ///< Response to socket connect
     XBEE_API_TYPE_CELLULAR_SOCKET_CLOSE_RESPONSE = 0xC3, ///< Response to socket close
     XBEE_API_TYPE_CELLULAR_SOCKET_BIND_RESPONSE = 0xC6, ///< Response to socket bind

     XBEE_API_TYPE_CELLULAR_SOCKET_RX = 0xCD,       ///< Frame for receiving data
     XBEE_API_TYPE_CELLULAR_SOCKET_RX_FROM = 0xCE,  ///< Frame for receiving UDP data with source
     XBEE_API_TYPE_CELLULAR_SOCKET_STATUS = 0xCF, ///< Frame for socket status
 
     /**< XBee GPIO/ADC Related API Frames */
     XBEE_API_TYPE_IO_DATA_SAMPLE_RX = 0x92,        ///< Frame for receiving IO data samples (GPIO/ADC)
     XBEE_API_TYPE_REMOTE_AT_COMMAND = 0x17,        ///< Frame for sending remote AT commands (GPIO/ADC)
     XBEE_API_TYPE_REMOTE_AT_RESPONSE = 0x97,       ///< Frame for receiving remote AT command responses (GPIO/ADC)
     XBEE_API_TYPE_IO_SAMPLE_RX_INDICATOR = 0x8F,   ///< Frame for receiving IO samples with explicit addressing (GPIO/ADC)
 
     // Add other types as needed
 
 } xbee_api_frame_type_t;
 
 
 /**
  * @enum api_receive_status_t
  * @brief Enumeration of status codes for receiving XBee API frames.
  *
  * This enum defines the possible status codes that can be returned by the 
  * `api_receive_api_frame` function. These status codes indicate whether the frame 
  * was successfully received or if there was an error, and if so, what type of error occurred.
  */
 typedef enum {
     API_RECEIVE_SUCCESS = 0,                  ///< Frame received successfully
     API_RECEIVE_ERROR_INVALID_POINTER = -1,   ///< Invalid frame pointer (NULL)
     API_RECEIVE_ERROR_TIMEOUT_START_DELIMITER = -2, ///< Timeout or error reading start delimiter
     API_RECEIVE_ERROR_INVALID_START_DELIMITER = -3, ///< Invalid start delimiter received
     API_RECEIVE_ERROR_TIMEOUT_LENGTH = -4,    ///< Timeout or error reading frame length
     API_RECEIVE_ERROR_FRAME_TOO_LARGE = -5,   ///< Frame length exceeds buffer size
     API_RECEIVE_ERROR_TIMEOUT_DATA = -6,      ///< Timeout or error reading frame data
     API_RECEIVE_ERROR_TIMEOUT_CHECKSUM = -7,  ///< Timeout or error reading checksum
     API_RECEIVE_ERROR_INVALID_CHECKSUM = -8,  ///< Invalid checksum detected
     API_RECEIVE_ERROR_UART_FAILURE = -9       ///< UART read or write failure
 } api_receive_status_t;
 
 /**
  * @struct xbee_api_frame_t
  * @brief Structure to represent an XBee API frame.
  *
  * This structure is used to represent an XBee API frame, which is a structured data packet 
  * exchanged between the host (e.g., microcontroller) and the XBee module. An API frame 
  * contains a specific type, a length field, a checksum for data integrity, and the actual 
  * frame data.
  *
  * XBee API frames are used for various communication tasks, including sending commands, 
  * transmitting data, receiving status information, and controlling IO pins. Each field 
  * in this structure serves a specific purpose in constructing and interpreting these frames.
  *
  * @var xbee_api_frame_t::type
  *     The type of the API frame, represented by the `xbee_api_frame_type_t` enumeration. 
  *     This field indicates the purpose of the frame, such as whether it is an AT command, 
  *     a data transmission, or a status report.
  * @var xbee_api_frame_t::length
  *     The length of the frame data, excluding the start delimiter, length bytes, and checksum.
  *     This field specifies the size of the data portion of the frame, helping in parsing and 
  *     processing the frame.
  * @var xbee_api_frame_t::checksum
  *     The checksum of the API frame, used to verify the integrity of the frame data. 
  *     The checksum is calculated over the frame's data and is essential for detecting 
  *     transmission errors.
  * @var xbee_api_frame_t::data
  *     The actual data contained within the API frame. This array holds the payload or 
  *     command data associated with the frame type. The size of this array is defined by 
  *     `XBEE_MAX_FRAME_DATA_SIZE`, which should be set according to the maximum expected 
  *     frame size in the application.
  *
  * Example Usage:
  * @code
  * xbee_api_frame_t frame;
  * frame.type = XBEE_API_TYPE_TX_REQUEST;
  * frame.length = 10;
  * frame.data[0] = 0x01; // Example payload data
  * frame.checksum = calculateChecksum(frame.data, frame.length);
  * @endcode
  */
 typedef struct {
     xbee_api_frame_type_t type;  ///< Type of the API frame
     uint16_t length;             ///< Length of the frame data
     uint8_t checksum;            ///< Checksum of the API frame
     uint8_t data[XBEE_MAX_FRAME_DATA_SIZE]; ///< Frame data
 } xbee_api_frame_t;
 
 
 // Function prototypes
 api_receive_status_t apiReceiveApiFrame(XBee* self, xbee_api_frame_t *frame);
 int apiSendAtCommand(XBee* self,at_command_t command, const uint8_t *parameter, uint8_t paramLength);
 int apiSendFrame(XBee* self,uint8_t frame_type, const uint8_t *data, uint16_t len);
 int apiSendAtCommandAndGetResponse(XBee* self, at_command_t command, const uint8_t *parameter, 
     uint8_t paramLength, uint8_t *responseBuffer, uint8_t *responseLength, uint32_t timeoutMs, uint16_t responseBufferSize);
 void apiHandleFrame(XBee* self,xbee_api_frame_t frame);
 void xbeeHandleAtResponse(XBee* self,xbee_api_frame_t *frame);
 void xbeeHandleModemStatus(XBee* self,xbee_api_frame_t *frame);
 void xbeeHandleRxPacket(XBee* self,xbee_api_frame_t *frame);
 int asciiToHexArray(const char *asciiStr, uint8_t *hexArray, size_t maxLen);
 
 #if defined(__cplusplus)
 }
 #endif
 
 #endif // XBEE_FRAMES_H

