/**
 * @file xbee_lr.h
 * @brief Header file for the XBee LR (LoRaWAN) subclass.
 * 
 * This file defines the interface for the XBee LR subclass, which extends the
 * functionality of the base XBee class to support LoRaWAN
 * communication features specific to XBee LR modules. It includes prototypes
 * for initialization, connection handling, and other LR-specific operations.
 * 
 * @version 1.1
 * @date 2024-04-04
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

 #ifndef XBEELR_H
 #define XBEELR_H
 
 #if defined(__cplusplus)
 extern "C"
 {
 #endif
 
 #include "xbee.h"
 #include "config.h"
 
 //Minimum Connection Timeout required for EU868 Join is 8000ms
 #define CONNECTION_TIMEOUT_MS 8000 
 #define SEND_DATA_TIMEOUT_MS 10000
 
 // Structure for XBee LR LoRaWAN packet
 typedef struct XBeeLRPacket_s{
     uint8_t port;
     uint8_t payloadSize;
     uint8_t *payload;
     uint8_t ack;
     uint8_t status;
     uint8_t frameId;
     //For RX only
     int8_t rssi;
     int8_t snr;
     uint8_t dr;
     uint8_t slot;
     uint32_t counter;
     //For TX only
     uint8_t channel;
     int8_t power;
 }XBeeLRPacket_t;
 
 // Subclass for XBeeLR
 typedef struct {
     XBee base;  // Inherit from XBee
     // Add XBeeLR specific attributes here like methods specific to an XBee type
 } XBeeLR;
 
 
 XBeeLR* XBeeLRCreate(const XBeeCTable* cTable, const XBeeHTable* hTable);
 bool XBeeLRGetDevEUI(XBee* self, char* responseBuffer, uint8_t buffer_size);
 bool XBeeLRSetAppEUI(XBee* self, const char* value);
 bool XBeeLRSetAppKey(XBee* self, const char* value);
 
 bool XBeeLRSetNwkKey(XBee* self, const char* value);
 bool XBeeLRSetClass(XBee* self, const char value);
 bool XBeeLRSetActivationMode(XBee* self, const uint8_t value);
 bool XBeeLRSetADR(XBee* self, const uint8_t value);
 bool XBeeLRSetDataRate(XBee* self, const uint8_t value);
 bool XBeeLRSetRegion(XBee* self, const uint8_t value);
 bool XBeeLRSetDutyCycle(XBee* self, const uint8_t value);
 bool XBeeLRGetSpecVersion(XBee* self,  char* responseBuffer, uint8_t buffer_size);
 bool XBeeLRSetJoinRX1Delay(XBee* self, const uint32_t value);
 bool XBeeLRSetJoinRX2Delay(XBee* self, const uint32_t value);
 bool XBeeLRSetRX1Delay(XBee* self, const uint32_t value);
 bool XBeeLRSetRX2Delay(XBee* self, const uint32_t value);
 bool XBeeLRSetRX2DataRate(XBee* self, const uint8_t value);
 bool XBeeLRSetRX2Frequency(XBee* self, const uint32_t value);
 bool XBeeLRSetTransmitPower(XBee* self, const uint8_t value);
 bool XBeeLRSetChannelsMask(XBee* self, const char* value);
 bool XBeeLRInit(XBee* self, uint32_t baudRate, void* device);
 bool XBeeLRConnected(XBee* self);
 uint8_t XBeeLRSendPacket(XBee* self, const void* data);
 
 #if defined(__cplusplus)
 }
 #endif
 
 #endif // XBEELR_H
