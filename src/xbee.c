/**
 * @file xbee.c
 * @brief Implementation of the XBee class.
 * 
 * This file contains the implementation of the core functions used to interact
 * with XBee modules using the API frame format. The functions provide an interface
 * for initializing the module, connecting/disconnect to the nework, 
 * sending and receiving data, and handling AT commands.
 * 
 * @version 1.2
 * @date 2025-05-31
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
**/

#include "xbee.h"
#include "xbee_api_frames.h" 

// Base class methods

/**
 * @brief Initializes the XBee module.
 * 
 * This function initializes the XBee module by setting the initial frame ID counter 
 * and calling the XBee subclass specific initialization routine.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[in] baudrate Baud rate for the serial communication.
 * @param[in] device Path to the serial device (e.g., "/dev/ttyUSB0").
 * 
 * @return True if initialization is successful, otherwise false.
 */
bool XBeeInit(XBee* self, uint32_t baudRate, void* device) {
    self->frameIdCntr = 1;
    return self->vtable->init(self, baudRate, device);
}

/**
 * @brief Connects the XBee to the network.
 * 
 * This function connects to the network by calling the XBee subclass specific 
 * connection implementation provided by the XBee subclass. This is a blocking function.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return True if the connection is successful, otherwise false.
 * @todo Add support for non-blocking connection attempts.
 */
bool XBeeConnect(XBee* self, bool blocking) {
    return self->vtable->connect(self, blocking);
}

/**
 * @brief Disconnects the XBee from the network.
 * 
 * This function closes the connection by calling the platform-specific 
 * close implementation provided by the XBee subclass. This is a blocking function.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return True if the disconnection is successful, otherwise false.
 */
bool XBeeDisconnect(XBee* self) {
    return self->vtable->disconnect(self);
}

/**
 * @brief Request XBee to send data over network.
 * 
 * This function sends data over the network by calling the XBee subclass specific 
 * send data implementation provided by the XBee subclass. This is a blocking function.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return xbee_deliveryStatus_t, 0 if successful
 */
uint8_t XBeeSendPacket(XBee* self, const void* data) {
    return self->vtable->sendData(self, data);
}

/**
 * @brief Performs a module reboot (ATRE).
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return bool True if the command was sent successfully, otherwise false.
 */
bool XBeeSoftReset(XBee* self){
    return apiSendAtCommand(self, AT_RE, NULL, 0) == API_SEND_SUCCESS;
}

/**
 * @brief Performs a hard reset of the XBee module.
 * 
 * This function invokes the `hard_reset` method defined in the XBee subclass's 
 * virtual table (vtable) to perform a hard reset of the XBee module. A hard reset 
 * usually involves a full power cycle or reset through rst pin, resetting the module completely.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return void This function does not return a value.
 */
void XBeeHardReset(XBee* self) {
    self->vtable->hardReset(self);
}

/**
 * @brief Calls the XBee subclass's process implementation.
 * 
 * This function invokes the `process` method defined in the XBee subclass's 
 * virtual table (vtable). It is responsible for processing any ongoing tasks 
 * or events related to the XBee module and must be called continuously in the 
 * application's main loop to ensure proper operation.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return void This function does not return a value.
 */
void XBeeProcess(XBee* self) {
    self->vtable->process(self);
}

/**
 * @brief Checks if the XBee module is connected to the network.
 * 
 * This function calls the `connected` method defined in the XBee subclass's 
 * virtual table (vtable) to determine if the XBee module is currently connected 
 * to the network. It returns true if the module is connected, otherwise false.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return bool Returns true if the XBee module is connected, otherwise false.
 */
bool XBeeConnected(XBee* self) {
    return self->vtable->connected(self);
}

/**
 * @brief Sends the ATWR command to write the current configuration to the XBee module's non-volatile memory.
 * 
 * This function sends the ATWR command using an API frame to write the current configuration settings 
 * to the XBee module's non-volatile memory. The function waits for a response from the module 
 * to confirm that the command was successful. If the command fails or the module does not respond, 
 * a debug message is printed.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return bool Returns true if the configuration was successfully written, otherwise false.
 */
bool XBeeWriteConfig(XBee* self) {
    uint8_t response[33];
    uint8_t responseLength;
    int status = apiSendAtCommandAndGetResponse(self, AT_WR, NULL, 0, response, &responseLength, 5000, sizeof(response));
    if(status != API_SEND_SUCCESS){
        XBEEDebugPrint("Failed to Write Config\n");
        return false;
    }
    return true;
}

/**
 * @brief Sends the ATAC command to apply pending configuration changes on the XBee module.
 * 
 * This function sends the ATAC command using an API frame to apply any pending configuration changes 
 * on the XBee module. The function waits for a response from the module to confirm that the command 
 * was successful. If the command fails or the module does not respond, a debug message is printed.
 * 
 * @param[in] self Pointer to the XBee instance.
 * 
 * @return bool Returns true if the changes were successfully applied, otherwise false.
 */

bool XBeeApplyChanges(XBee* self) {
    uint8_t response[33];
    uint8_t responseLength;
    int status = apiSendAtCommandAndGetResponse(self, AT_AC, NULL, 0, response, &responseLength, 5000, sizeof(response));
    if(status != API_SEND_SUCCESS){
        XBEEDebugPrint("Failed to Apply Changes\n");
        return false;
    }
    return true;
}

/**
 * @brief Sends the AT_AO command to set API Options.
 * 
 * This function configures the API Options on the XBee module 
 * by sending the AT command `AT_AO` with the specified API Options value. The function 
 * is blocking, meaning it waits for a response from the module or until a timeout 
 * occurs. If the command fails to send or the module does not respond, a debug 
 * message is printed.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[in] value The API Options to be set, provided as a string.
 * 
 * @return bool Returns true if the API Options was successfully set, otherwise false.
 */
bool XBeeSetAPIOptions(XBee* self, const uint8_t value) {
    uint8_t response[33];
    uint8_t responseLength;
    int status = apiSendAtCommandAndGetResponse(self, AT_AO, (const uint8_t[]){value}, 1, response, &responseLength, 5000, sizeof(response));
    if(status != API_SEND_SUCCESS){
        XBEEDebugPrint("Failed to set API Options\n");
        return false;
    }
    return true;
}

/**
 * @brief Configures the XBee module with protocol-specific parameters.
 * 
 * This function delegates configuration to the subclass via the virtual table.
 * It is optional and can be NULL if not needed by the specific module.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[in] config Pointer to protocol-specific configuration structure.
 * 
 * @return bool Returns true if configuration was accepted, false if unsupported or failed.
 */
bool XBeeConfigure(XBee* self, const void* config) {
    if (self->vtable->configure) {
        return self->vtable->configure(self, config);
    }
    XBEEDebugPrint("Configure() not supported for this module.\n");
    return false;
}

/**
 * @brief Retrieves the firmware version of the XBee module using the ATVR command.
 * 
 * This function sends the AT_VR command and returns the firmware version as a 32-bit integer.
 * It assumes the version is returned as a 4-byte value.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[out] version Pointer to store the retrieved 32-bit firmware version.
 * 
 * @return bool Returns true if the firmware version was retrieved successfully.
 */
bool XBeeGetFirmwareVersion(XBee* self, uint32_t* version) {
    if (!version) return false;

    uint8_t response[4];
    uint8_t responseLength = 0;
    int status = apiSendAtCommandAndGetResponse(self, AT_VR, NULL, 0, response, &responseLength, 5000, sizeof(response));

    if (status != API_SEND_SUCCESS || responseLength != 4) {
        XBEEDebugPrint("Failed to retrieve firmware version (ATVR)\n");
        return false;
    }

    *version = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3];
    return true;
}

/**
 * @brief Restores factory defaults (ATFR).
 *
 * Sends the `AT_FR` command and returns when the frame is accepted.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return bool True if the command was sent successfully, otherwise false.
 */
bool XBeeFactoryReset(XBee* self)
{
    return apiSendAtCommand(self, AT_FR, NULL, 0) == API_SEND_SUCCESS;
}

/**
 * @brief Performs a module reboot (ATRE).
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return bool True if the command was sent successfully, otherwise false.
 */
bool XBeeSoftRestart(XBee* self){
    return apiSendAtCommand(self, AT_RE, NULL, 0) == API_SEND_SUCCESS;
}

/**
 * @brief Exits AT-command mode (ATCN).
 *
 * Useful after using legacy “transparent” `+++` mode.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return bool True if the command was sent successfully, otherwise false.
 */
bool XBeeExitCommandMode(XBee* self){
    return apiSendAtCommand(self, AT_CN, NULL, 0) == API_SEND_SUCCESS;
}

/**
 * @brief Enables or disables API mode (ATAP).
 *
 * @param[in] self  Pointer to the XBee instance.
 * @param[in] mode  0 = Transparent, 1 = API (no escape), 2 = API-escaped.
 *
 * @return bool True if the command was accepted, otherwise false.
 */
bool XBeeSetApiEnable(XBee* self, uint8_t mode){
    return apiSendAtCommand(self, AT_AP, &mode, 1) == API_SEND_SUCCESS;
}

/**
 * @brief Changes the UART baud-rate (ATBD).
 *
 * Pass the *rate code* defined by Digi (e.g. 3 → 9600, 7 → 115200).
 *
 * @param[in] self      Pointer to the XBee instance.
 * @param[in] rateCode  Digi baud-rate code.
 *
 * @return bool True if the baud-rate was accepted, otherwise false.
 */
bool XBeeSetBaudRate(XBee* self, uint8_t rateCode){
    return apiSendAtCommand(self, AT_BD, &rateCode, 1) == API_SEND_SUCCESS;
}

/**
 * @brief Reads last-hop RSSI in dBm (ATDB).
 *
 * @param[in]  self     Pointer to the XBee instance.
 * @param[out] rssiOut  Pointer that receives signed RSSI in dBm.
 *
 * @return bool True if RSSI was read successfully, otherwise false.
 */
bool XBeeGetLastRssi(XBee* self, int8_t* rssiOut){
    if (!rssiOut) return false;

    uint8_t resp;
    uint8_t len = 0;

    if (apiSendAtCommandAndGetResponse(self, AT_DB, NULL, 0,
                                       &resp, &len, 2000, sizeof(resp)) != API_SEND_SUCCESS || len != 1)
    {
        XBEEDebugPrint("Failed to read RSSI (ATDB)\n");
        return false;
    }

    *rssiOut = -(int8_t)resp;   /* Digi returns a positive offset */
    return true;
}

/**
 * @brief Retrieves the hardware revision (ATHV).
 *
 * @param[in]  self    Pointer to the XBee instance.
 * @param[out] hvOut   Pointer to receive 16-bit hardware version.
 *
 * @return bool True if the hardware version was retrieved, otherwise false.
 */
bool XBeeGetHardwareVersion(XBee* self, uint16_t* hvOut){
    if (!hvOut) return false;

    uint8_t resp[2];
    uint8_t len = 0;

    if (apiSendAtCommandAndGetResponse(self, AT_HV, NULL, 0,
                                       resp, &len, 2000, sizeof(resp)) != API_SEND_SUCCESS || len != 2)
    {
        XBEEDebugPrint("Failed to retrieve hardware version (ATHV)\n");
        return false;
    }

    *hvOut = (uint16_t)resp[0] << 8 | resp[1];
    return true;
}

/**
 * @brief Retrieves the 64-bit factory serial number (ATSH + ATSL).
 *
 * @param[in]  self    Pointer to the XBee instance.
 * @param[out] snOut   Pointer to receive the 64-bit serial number.
 *
 * @return bool True if the serial number was retrieved, otherwise false.
 */
bool XBeeGetSerialNumber(XBee* self, uint64_t* snOut){
    if (!snOut) return false;

    uint8_t hi[4], lo[4];
    uint8_t lenHi = 0, lenLo = 0;

    if (apiSendAtCommandAndGetResponse(self, AT_SH, NULL, 0,
                                       hi, &lenHi, 2000, sizeof(hi)) != API_SEND_SUCCESS || lenHi != 4)
    {
        XBEEDebugPrint("Failed to retrieve serial high (ATSH)\n");
        return false;
    }

    if (apiSendAtCommandAndGetResponse(self, AT_SL, NULL, 0,
                                       lo, &lenLo, 2000, sizeof(lo)) != API_SEND_SUCCESS || lenLo != 4)
    {
        XBEEDebugPrint("Failed to retrieve serial low (ATSL)\n");
        return false;
    }

    *snOut = ((uint64_t)hi[0] << 56) | ((uint64_t)hi[1] << 48) |
             ((uint64_t)hi[2] << 40) | ((uint64_t)hi[3] << 32) |
             ((uint64_t)lo[0] << 24) | ((uint64_t)lo[1] << 16) |
             ((uint64_t)lo[2] <<  8) |  (uint64_t)lo[3];

    return true;
}