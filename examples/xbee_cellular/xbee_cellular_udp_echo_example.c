/***************************************************************************//**
 *  @file xbee_cellular_udp_echo_example.c
 *
 *  @brief Sends a UDP datagram to Digi’s public echo server (52.43.121.77:10001)
 *         and prints the echoed payload when it comes back.
 *
 *  The flow implements the Extended-Socket “UDP” example:
 *    1)  Socket Create   (0x40, protocol 0x02)
 *    2)  Socket Bind     (0x46, choose a local port)
 *    3)  Socket SendTo   (0x45)                --> TX-Status (0x89)
 *    4)  Socket ReceiveFrom (0xCE)             --> our RX callback
 *    5)  Socket Close    (0x43)                --> Close-Resp (0xCF)
 *
 *  @author
 *    Felix Galindo <felix.galindo@digi.com>
 *  @license
 *    MIT
 ******************************************************************************/

#include "xbee_cellular.h"
#include "port.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* ---------- serial device per-platform ---------------------------------- */
#if defined(_WIN32)
  #define DEFAULT_SERIAL_PORT "COM3"
#elif defined(__APPLE__)
  #define DEFAULT_SERIAL_PORT "/dev/cu.usbserial-1110"
#elif defined(__linux__)
  #define DEFAULT_SERIAL_PORT "/dev/ttyUSB0"
#else
  #define DEFAULT_SERIAL_PORT ""
#endif

/* ---------- globals ------------------------------------------------------ */
static volatile bool echoReceived = false;

/* ---------- user callbacks ---------------------------------------------- */
static void onReceive(XBee *self, void *frame)
{
    (void)self;
    const XBeeCellularPacket_t *p = (const XBeeCellularPacket_t *)frame;

    echoReceived = true;

    portDebugPrintf("[UDP RX] %u bytes from %u.%u.%u.%u:%u (socket %u):\n",
                    p->payloadSize,
                    p->ip[0], p->ip[1], p->ip[2], p->ip[3],
                    p->remotePort, p->socketId);

    for (uint16_t i = 0; i < p->payloadSize; ++i) {
        char c = p->payload[i];
        portDebugPrintf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    portDebugPrintf("\n");
}

static void onSend(XBee *self, void *frame)
{
    (void)self;
    (void)frame;
    portDebugPrintf("[UDP TX] payload accepted for transmit\n");
}

/* ---------- main --------------------------------------------------------- */
int main(void)
{
    /* Hardware abstraction table ------------------------------------------- */
    const XBeeHTable hw = {
        .PortUartInit  = portUartInit,
        .PortUartRead  = portUartRead,
        .PortUartWrite = portUartWrite,
        .PortFlushRx   = portFlushRx,
        .PortDelay     = portDelay,
        .PortMillis    = portMillis
    };

    /* Callback table ------------------------------------------------------- */
    const XBeeCTable cb = {
        .OnReceiveCallback = onReceive,
        .OnSendCallback    = onSend
    };

    portDebugPrintf("XBee 3 Cellular – UDP Echo example (Extended Socket)\n");

    /* Instance ------------------------------------------------------------- */
    XBeeCellular *xbee = XBeeCellularCreate(&cb, &hw);

    if (!XBeeInit((XBee *)xbee, 9600, DEFAULT_SERIAL_PORT)) {
        portDebugPrintf("[ERR] failed to open serial port\n");
        return -1;
    }

    /* APN etc. – identical to the HTTP example ---------------------------- */
    const XBeeCellularConfig_t cfg = {
        .apn     = "broadband",
        .simPin  = "",
        .carrier = ""
    };
    XBeeConfigure((XBee *)xbee, &cfg);

    /* Attach to network ---------------------------------------------------- */
    portDebugPrintf("Waiting for network attach...\n");
    XBeeConnect((XBee *)xbee, false);
    while (!XBeeCellularConnected((XBee *)xbee)) {
        portDelay(1000);
        portDebugPrintf(".");
    }
    portDebugPrintf("\n[OK] attached!\n");

    /* 1) Socket Create (protocol = UDP 0x02) ------------------------------- */
    uint8_t sockId = 0;
    if (!XBeeCellularSocketCreate((XBee *)xbee, XBEE_PROTOCOL_UDP, &sockId)) {
        portDebugPrintf("[ERR] socket create failed\n");
        return -1;
    }

    /* 2) Bind to local port 0x1234 ---------------------------------------- */
    if (!XBeeCellularSocketBind((XBee *)xbee, sockId, 0x1234, true)) {
        portDebugPrintf("[ERR] bind failed\n");
        return -1;
    }
    portDebugPrintf("[OK] socket %u bound to local port 0x1234\n", sockId);

    /* 3) SendTo Digi echo server ------------------------------------------ */
    const uint8_t ip[4] = { 52, 43, 121, 77 };
    const uint8_t payload[] = "echo this";

    if (!XBeeCellularSocketSendTo((XBee *)xbee,
                                  sockId,
                                  ip,
                                  10001,
                                  payload,
                                  sizeof(payload) - 1))
    {
        portDebugPrintf("[ERR] sendto failed\n");
        return -1;
    }
    portDebugPrintf("[OK] UDP datagram sent, waiting for echo...\n");

    /* 4) Wait (≤10 s) for echo --------------------------------------------- */
    uint32_t t0 = portMillis();
    while (!echoReceived && (portMillis() - t0) < 10000) {
        XBeeProcess((XBee *)xbee);
        portDelay(100);
    }

    if (!echoReceived)
        portDebugPrintf("[WARN] timed-out waiting for echo\n");

    /* 5) Close socket ------------------------------------------------------ */
    XBeeCellularSocketClose((XBee *)xbee, sockId, false);

    XBeeCellularDestroy(xbee);
    portDebugPrintf("Done.\n");
    return 0;
}