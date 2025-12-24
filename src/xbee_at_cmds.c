/**
 * @file xbee_at_cmds.c
 * @brief Implementation of XBee AT commands.
 * 
 * This file contains the implementation of functions used to handle and send AT commands
 * to the XBee module. 
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

 #include "xbee_at_cmds.h"
 #include "port.h"
 #include <stdio.h>
 #include <string.h>
 
 // AT Command Functions
 
 /**
  * @brief Converts an AT command enum to its string representation.
  * 
  * This function takes an AT command enum value and returns the corresponding
  * string representation of the command.
  * 
  * @param[in] command The AT command enum value.
  * 
  * @return The string representation of the AT command.
  */
 const char* atCommandToString(at_command_t command) {
     switch (command) {
         /**< XBee Common AT Commands */
         case AT_: return "AT";              ///< Placeholder for unspecified commands
         case AT_CN: return "CN";            ///< Exit Command Mode
         case AT_AP: return "AP";            ///< API Enable
         case AT_BD: return "BD";            ///< Baud Rate
         case AT_WR: return "WR";            ///< Write to non-volatile memory
         case AT_RE: return "RE";            ///< Soft Reset
         case AT_FR: return "FR";            ///< Factory Reset
         case AT_VR: return "VR";            ///< Firmware Version
         case AT_AC: return "AC";            ///< Apply Changes
         case AT_NR: return "NR";            ///< Network Reset
         case AT_DD: return "DD";            ///< Device Type Identifier
         case AT_ID: return "ID";            ///< PAN ID
         case AT_NI: return "NI";            ///< Node Identifier
         case AT_DL: return "DL";            ///< Destination Address Low
         case AT_DH: return "DH";            ///< Destination Address High
         case AT_SH: return "SH";            ///< Serial Number High
         case AT_SL: return "SL";            ///< Serial Number Low
         case AT_PL: return "PL";            ///< Power Level
         case AT_AI: return "AI";            ///< Association Indication
         case AT_RP: return "RP";            ///< RSSI PWM Timer
         case AT_RN: return "RN";            ///< Random Delay Slots
         case AT_RR: return "RR";            ///< Retries
         case AT_ND: return "ND";            ///< Node Discover
         case AT_NO: return "NO";            ///< Network Discovery Options
         case AT_RO: return "RO";            ///< Packetization Timeout
         case AT_SM: return "SM";            ///< Sleep Mode
         case AT_SO: return "SO";            ///< Sleep Options
         case AT_SP: return "SP";            ///< Sleep Period
         case AT_ST: return "ST";            ///< Time Before Sleep
         case AT_IS: return "IS";            ///< Force Sample (IO)
         case AT_P0: return "P0";            ///< DIO0/AD0 Configuration
         case AT_P1: return "P1";            ///< DIO1/AD1 Configuration
         case AT_P2: return "P2";            ///< DIO2/AD2 Configuration
         case AT_P3: return "P3";            ///< DIO3/AD3 Configuration
         case AT_P4: return "P4";            ///< DIO4 Configuration
         case AT_P5: return "P5";            ///< DIO5 Configuration
         case AT_P6: return "P6";            ///< DIO6 Configuration
         case AT_P7: return "P7";            ///< DIO7 Configuration
         case AT_P8: return "P8";            ///< DIO8 Configuration
         case AT_PR: return "PR";            ///< Pull-up Resistor Enable
         case AT_RI: return "RI";            ///< Ring Indicator
         case AT_CT: return "CT";            ///< Command Mode Timeout
         case AT_GT: return "GT";            ///< Guard Times
         case AT_SB: return "SB";            ///< Stop Bits
         case AT_D7: return "D7";            ///< DIO7 Configuration
         case AT_D8: return "D8";            ///< DIO8 Configuration
         case AT_D9: return "D9";            ///< DIO9 Configuration
         case AT_DA: return "DA";            ///< DIO10 Configuration
         case AT_DB: return "DB";            ///< RSSI for Last Hop
         case AT_DC: return "DC";            ///< DIO Change Detect
         case AT_FT: return "FT";            ///< Flow Control Threshold
         case AT_GU: return "GU";            ///< DIO Pull-up Resistor Enable
         case AT_HS: return "HS";            ///< Hardware Sleep Control
         case AT_IT: return "IT";            ///< RSSI Timer
         case AT_NJ: return "NJ";            ///< Node Join Time
         case AT_JN: return "JN";            ///< Join Notification
         case AT_JT: return "JT";            ///< Join Time
         case AT_JV: return "JV";            ///< Channel Verification
         case AT_LD: return "LD";            ///< Node Discovery Time
         case AT_AO: return "AO";            ///< API Options
         case AT_HV: return "HV";            ///< Hardware Version

 
         /**< XBee 3 RF Specific AT Commands */
         case AT_CE: return "CE";            ///< Coordinator Enable
         case AT_SE: return "SE";            ///< Source Endpoint
         case AT_DE_RF: return "DE";         ///< Destination Endpoint (RF specific)
         case AT_CI: return "CI";            ///< Cluster Identifier
         case AT_BH: return "BH";            ///< Broadcast Hops
         case AT_YS: return "YS";            ///< Sleep Status
         case AT_WR_RF: return "WR";         ///< Write to non-volatile memory (RF specific)
 
         /**< XBee 3 Cellular Specific AT Commands */
         case AT_IP: return "IP";            ///< IP Address
         case AT_MA: return "MA";            ///< MAC Address
         case AT_OK: return "OK";            ///< Cellular OK Command
         case AT_RI_CELL: return "RI";       ///< Ring Indicator (Cellular Specific)
         case AT_SR: return "SR";            ///< Serial Number
         case AT_TD: return "TD";            ///< Transmit Delay
         case AT_TR: return "TR";            ///< Transmission Retry Count
         case AT_TS: return "TS";            ///< Transmission Status
         case AT_UK: return "UK";            ///< Unlock Password
         case AT_VE: return "VE";            ///< Voltage Supply
         case AT_VL: return "VL";            ///< Cellular Module Version
         case AT_PN: return "PN";     ///< SIM PIN
         case AT_AN: return "AN";     ///< APN
         case AT_CP: return "CP";     ///< Carrier Profile
         case AT_SD: return "SD";     ///< Shutdown
        
         /**< XBee LR Specific AT Commands */
         case AT_DE: return "DE";            ///< LoRaWAN Device EUI
         case AT_AK: return "AK";            ///< LoRaWAN Application Key
         case AT_AE: return "AE";            ///< LoRaWAN Application EUI
         case AT_NK: return "NK";            ///< LoRaWAN Network Key
         case AT_JS: return "JS";            ///< LoRaWAN Join Status
         case AT_LC: return "LC";            ///< LoRaWAN Class
         case AT_AM: return "AM";            ///< LoRaWAN Activation Mode
         case AT_AD: return "AD";            ///< LoRaWAN ADR
         case AT_DR: return "DR";            ///< LoRaWAN DataRate
         case AT_LR: return "LR";            ///< LoRaWAN Region
        // case AT_DC: return "DC";            ///< LoRaWAN Duty Cycle        
         case AT_LV: return "LV";            ///< LoRaWAN Spec Version     
         case AT_J1: return "J1";            ///< LoRaWAN Join RX1 Delay        
         case AT_J2: return "J2";            ///< LoRaWAN Join RX2 Delay
         case AT_D1: return "D1";            ///< LoRaWAN RX1 Delay
         case AT_D2: return "D2";            ///< LoRaWAN RX2 Delay
         case AT_XD: return "XD";            ///< LoRaWAN RX2 Data Rate
         case AT_XF: return "XF";            ///< LoRaWAN RX2 Frequency   
         case AT_PO: return "PO";            ///< LoRaWAN Transmit Power   
         case AT_CM: return "CM";            ///< LoRaWAN Channels Mask
 
         default: return NULL;               ///< Unknown command
     }
 }
 
 
 