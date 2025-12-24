/**
 * @file xbee_at_cmds.h
 * @brief Header file for XBee AT commands.
 * 
 * This file defines the interface for working with XBee AT commands. AT commands
 * are used for configuring and controlling the XBee module. 
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


#ifndef XBEE_AT_H
#define XBEE_AT_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stdint.h>
#include "config.h"

/**
 * @enum at_command_t
 * @brief Enumeration of common and module-specific XBee AT commands.
 *
 * This enum defines the AT commands supported by various XBee modules. Each command
 * corresponds to a specific configuration or control operation within the module.
 *
 * AT commands allow users to configure the XBee module, control its behavior,
 * and query various parameters such as network status, signal strength, and device settings.
 */
typedef enum {
    /**< XBee Common AT Commands */
    AT_,     /**< Placeholder for unspecified commands */
    AT_CN,   /**< Exit Command Mode */
    AT_AP,   /**< API Enable */
    AT_BD,   /**< Baud Rate */
    AT_WR,   /**< Write to non-volatile memory */
    AT_RE,   /**< Soft Reset */
    AT_FR,   /**< Factory Reset */
    AT_VR,   /**< Firmware Version */
    AT_AC,   /**< Apply Changes */
    AT_NR,   /**< Network Reset */
    AT_DD,   /**< Device Type Identifier */
    AT_ID,   /**< PAN ID */
    AT_NI,   /**< Node Identifier */
    AT_DL,   /**< Destination Address Low */
    AT_DH,   /**< Destination Address High */
    AT_SH,   /**< Serial Number High */
    AT_SL,   /**< Serial Number Low */
    AT_PL,   /**< Power Level */
    AT_AI,   /**< Association Indication */
    AT_RP,   /**< RSSI PWM Timer */
    AT_RN,   /**< Random Delay Slots */
    AT_RR,   /**< Retries */
    AT_ND,   /**< Node Discover */
    AT_NO,   /**< Network Discovery Options */
    AT_RO,   /**< Packetization Timeout */
    AT_SM,   /**< Sleep Mode */
    AT_SO,   /**< Sleep Options */
    AT_SP,   /**< Sleep Period */
    AT_ST,   /**< Time Before Sleep */
    AT_IS,   /**< Force Sample (IO) */
    AT_P0,   /**< DIO0/AD0 Configuration */
    AT_P1,   /**< DIO1/AD1 Configuration */
    AT_P2,   /**< DIO2/AD2 Configuration */
    AT_P3,   /**< DIO3/AD3 Configuration */
    AT_P4,   /**< DIO4 Configuration */
    AT_P5,   /**< DIO5 Configuration */
    AT_P6,   /**< DIO6 Configuration */
    AT_P7,   /**< DIO7 Configuration */
    AT_P8,   /**< DIO8 Configuration */
    AT_PR,   /**< Pull-up Resistor Enable */
    AT_RI,   /**< Ring Indicator */
    AT_CT,   /**< Command Mode Timeout */
    AT_GT,   /**< Guard Times */
    AT_SB,   /**< Stop Bits */
    AT_D7,   /**< DIO7 Configuration */
    AT_D8,   /**< DIO8 Configuration */
    AT_D9,   /**< DIO9 Configuration */
    AT_DA,   /**< DIO10 Configuration */
    AT_DB,   /**< RSSI for Last Hop */
    AT_DC,   /**< DIO Change Detect */
    AT_FT,   /**< Flow Control Threshold */
    AT_GU,   /**< DIO Pull-up Resistor Enable */
    AT_HS,   /**< Hardware Sleep Control */
    AT_IT,   /**< RSSI Timer */
    AT_NJ,   /**< Node Join Time */
    AT_JN,   /**< Join Notification */
    AT_JT,   /**< Join Time */
    AT_JV,   /**< Channel Verification */
    AT_LD,   /**< Node Discovery Time */
    AT_AO,   /**< API Options */
    AT_HV,   /**< Hardware Version */
    //@todo: move xbee familiar specific commands to their own file
    
    /**< XBee 3 RF Specific AT Commands */
    AT_CE,   /**< Coordinator Enable */
    AT_SE,   /**< Source Endpoint */
    AT_DE_RF,/**< Destination Endpoint (RF specific) */
    AT_CI,   /**< Cluster Identifier */
    AT_BH,   /**< Broadcast Hops */
    AT_YS,   /**< Sleep Status */
    AT_WR_RF,/**< Write to non-volatile memory (RF specific) */

    /**< XBee 3 Cellular Specific AT Commands */
    AT_IP,   /**< IP Address */
    AT_MA,   /**< MAC Address */
    AT_OK,   /**< Cellular OK Command */
    AT_RI_CELL, /**< Ring Indicator (Cellular Specific) */
    AT_SR,   /**< Serial Number */
    AT_TD,   /**< Transmit Delay */
    AT_TR,   /**< Transmission Retry Count */
    AT_TS,   /**< Transmission Status */
    AT_UK,   /**< Unlock Password */
    AT_VE,   /**< Voltage Supply */
    AT_VL,   /**< Cellular Module Version */
    AT_SD,   /**< Start Data Session (Cellular specific) */
    AT_PN,   /**< SIM PIN */
    AT_AN,   /**< Access Point Name (APN) */
    AT_CP,    /**< Carrier Profile */


    /**< XBee LR Specific AT Commands */
    AT_DE,   /**< LoRaWAN Device EUI */
    AT_AK,   /**< LoRaWAN Application Key */
    AT_AE,   /**< LoRaWAN Application EUI */
    AT_NK,   /**< LoRaWAN Network Key */
    AT_JS,   /**< LoRaWAN Join Status */
    AT_LC,   /**< LoRaWAN Class */
    AT_AM,   /**< LoRaWAN Activation Mode */
    AT_AD,   /**< LoRaWAN ADR */
    AT_DR,   /**< LoRaWAN DataRate */
    AT_LR,   /**< LoRaWAN Region */
    //AT_DC,   /**< LoRaWAN Duty Cycle */
    AT_LV,   /**< LoRaWAN Spec Version */
    AT_J1,   /**< LoRaWAN Join RX1 Delay */
    AT_J2,   /**< LoRaWAN Join RX2 Delay */
    AT_D1,   /**< LoRaWAN RX1 Delay */
    AT_D2,   /**< LoRaWAN RX2 Delay */
    AT_XD,   /**< LoRaWAN RX2 Data Rate */ 
    AT_XF,   /**< LoRaWAN RX2 Frequency */ 
    AT_PO,   /**< LoRaWAN Transmit Power */ 
    AT_CM,   /**< LoRaWAN Channels Mask */

    // ... (other existing AT commands) ...

} at_command_t;


/**
 * @brief Converts an AT command enum to its corresponding string representation.
 *
 * This function takes an AT command enum and returns the corresponding string representation
 * that can be sent to the XBee module.
 *
 * @param command The AT command enum to convert.
 * @return const char* The string representation of the AT command.
 */
const char* atCommandToString(at_command_t command);

#if defined(__cplusplus)
}
#endif

#endif // XBEE_AT_H

