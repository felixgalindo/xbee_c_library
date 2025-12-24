
# XBee C Library

### Overview
This library provides a framework for interfacing with XBee modules using an XBee Library API that abstracts AT commands and API frames. It supports multiple platforms including Arduino, Unix, STM32, RP2350, and EFM32.

### Currently Supported XBees
- XBee LR (LoRaWAN)
- XBee 3 Cellular (LTE-M/NB-IoT)

### Library Structure
- **src**: Contains the core source files implementing XBee classes and APIs.
- **include**: Contains header files exposing the library's API.
- **examples**: Contains example implementations for various platforms.
- **test**: Contains unit tests for validating the functionality of the library.

### How to Run Examples
1. Choose the example that matches your platform (e.g., Unix, STM32).
2. Compile the example using your platform's toolchain.
3. Run the compiled binary to see the XBee communication in action.

### How to Run Unit Tests
1. Navigate to the `test` directory.
2. Compile the test files using your platform's toolchain.
3. Run the compiled binary to execute the unit tests.

### How to Add Support for Other XBee Subclasses
1. Extend the XBee API, API Frames, and AT commands files as needed for the new subclass.
2. Add the necessary support functions in the source files.
3. Update the examples to demonstrate the new functionality.

### Detailed File Breakdown
- **xbee.c**: Implements the XBee class
- **xbee_at_cmds.c**: Implements functions for sending and receiving AT commands.
- **xbee_lr.c**: Implements XBee LR module subclass.
- **xbee_api_frames.c**: Implements parsing and handling of API frames.

### Library Architecture
The library is designed to be modular, allowing easy expansion and support for different XBee modules and platforms. The main components include:
- **XBee Core**: Handles core XBee Class and API layer.
- **AT Commands**: Manages AT command interface.
- **API Frames**: Handles API frame communication.

### License 
See MIT-LICENSE.txt file

## Overview of the XBee Class Methods

This section provides an overview of the key methods available in the XBee class, which are used to interact with XBee modules.

- `XBeeInit()`: Initializes the XBee module.
- `XBeeConnect()`: Connects the XBee module to a network.
- `XBeeDisconnect()`: Disconnects the XBee module from the network.
- `XBeeSendPacket()`: Sends data through the XBee module.
- `XBeeSoftReset()`: Performs a soft reset on the XBee module.
- `XBeeHardReset()`: Performs a hard reset on the XBee module.
- `XBeeProcess()`: Processes incoming and outgoing data for the XBee module.
- `XBeeConnected()`: Checks if the XBee module is connected to a network.
- `XBeeWriteConfig()`: Writes configuration settings to the XBee module.
- `XBeeApplyChanges()`: Applies changes to the configuration of the XBee module.
- `XBeeLRSetApiOptions()`: Sets API options for long-range communication.

---

## XBee 3 Cellular Specific Methods

The following methods are specific to the XBee 3 Cellular subclass and are **not inherited** from the parent `XBee` class:

- `XBeeCellularCreate()`: Allocates and initializes an `XBeeCellular` instance with user-defined hardware and callback tables.
- `XBeeCellularDestroy()`: Frees memory associated with an `XBeeCellular` instance.
- `XBeeCellularConfigure()`: Applies APN, SIM PIN, and carrier profile settings to configure cellular behavior.
- `XBeeCellularSocketCreate()`: Sends a SOCKET_CREATE frame to open a new socket.
- `XBeeCellularSocketConnect()`: Connects a socket to a given remote address using DNS or IP.
- `XBeeCellularSocketSend()`: Transmits binary data over a connected socket.
- `XBeeCellularSocketSetOption()`: Configures socket parameters such as port binding or listen mode.
- `XBeeCellularSocketClose()`: Closes a previously created socket by sending a SOCKET_CLOSE frame.
- `XBeeCellularHandleRxPacket()`: Handles frame type `0xCD` (Socket Receive) and delivers received packets via the registered receive callback.

---

## XBee LR (LoRaWAN) Specific Methods

The following methods are specific to the XBee LR (LoRaWAN) subclass and are **not inherited** from the parent `XBee` class:

- `XBeeLRGetDevEUI()`: Reads and returns the LoRaWAN device EUI from the XBee module.
- `XBeeLRSetAppEUI()`: Configures the Application EUI used for OTAA join.
- `XBeeLRSetAppKey()`: Sets the AppKey for LoRaWAN OTAA authentication.
- `XBeeLRSetNwkKey()`: Sets the Network Key used for network traffic encryption.
- `XBeeLRSendPacket()`: Sends a LoRaWAN uplink packet using the LR frame interface.

Each of these methods provides essential functionality for managing and communicating with XBee devices within a network. Ensure that you refer to these methods when developing applications that involve XBee modules.

## Usage Example: XBee LR Example 


This section provides an example of how to use the XBee class, specifically for the XBee LR (LoRaWAN) module. The example covers creating an instance of the XBee LR class, initializing the module, setting up the hardware and command tables, configuring network settings, connecting to a network, sending data, and handling received data.

### Creating the XBee LR Instance

To create an instance of the XBee LR class, you need to pass the hardware and command tables to the `XBeeLRCreate` function:

```c
#include "xbee_lr.h"
#include "port.h"

// Define a pointer for the XBee LR instance
XBeeLR* myXbeeLr;

// Hardware Abstraction Function Pointer Table for XBeeLR
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
    .OnReceiveCallback = OnReceiveCallback, // Callback for receiving data
    .OnSendCallback = NULL,                 // Can be left as NULL if no callbacks are needed
};

int main() {
    // Create the XBee LR instance with hardware and command tables
    myXbeeLr = XBeeLRCreate(&XBeeLRHTable, &XBeeLRCTable);
    if (myXbeeLr == NULL) {
        printf("Failed to create XBee LR instance.");
        return -1;
    }

    // Proceed with initialization and connection...
}
```

### Initialization, Configuration, and Connection

After creating the XBee LR instance, initialize the XBee LR module, configure the network settings, and connect to the network:

```c
    // Initialize the XBee LR module
    if (!XBeeLRInit(myXbeeLr, 9600, "COM3")) {  // Replace "COM3" with the appropriate serial port for your platform
        printf("Failed to initialize XBee LR module.
");
        return -1;
    }

    // Read LoRaWAN DevEUI and print it
    uint8_t devEui[17];
    XBeeLRGetDevEUI((XBee*)myXbeeLr, devEui, sizeof(devEui));
    portDebugPrintf("DEVEUI: %s", devEui);

    // Set LoRaWAN Network Settings
    portDebugPrintf("Configuring...");
    XBeeLRSetAppEUI((XBee*)myXbeeLr, "37D56A3F6CDCF0A5");
    XBeeLRSetAppKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeLRSetNwkKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeWriteConfig((XBee*)myXbeeLr);
    XBeeApplyChanges((XBee*)myXbeeLr);

    // Connect to the LoRaWAN network
    portDebugPrintf("Connecting...");
    XBeeConnect((XBee*)myXbeeLr);

    printf("XBee LR module initialized and connected.");

    // Proceed with other operations...
```

### Sending Data

To send data over the XBee LR network, use the `XBeeLRSendPacket` method. Here's an example of preparing and sending a payload:

```c
// XBeeLR payload to send
uint8_t examplePayload[] = {0xC0, 0xC0, 0xC0, 0xFF, 0xEE};
uint16_t payloadLen = sizeof(examplePayload) / sizeof(examplePayload[0]);
XBeeLRPacket_t packet = {
    .payload = examplePayload,
    .payloadSize = payloadLen,
    .port = 2,
    .ack = 0,
};

if (!XBeeSendPacket(myXbeeLr, &packet)) {
    printf("Failed to send data.");
} else {
    printf("Data sent successfully.");
}
```

### Receiving Data

Handle data received from the XBee LR module using the `OnReceiveCallback` function:

```c
void OnReceiveCallback(XBee* self, void* data){
    XBeeLRPacket_t* packet = (XBeeLRPacket_t*) data;
    portDebugPrintf("Received Packet: ");
    for (int i = 1; i < packet->payloadSize; i++) {
        portDebugPrintf("%02X ", packet->payload[i]);
    }
    portDebugPrintf("");
    portDebugPrintf("Ack %u", packet->ack);
    portDebugPrintf("Port %u", packet->port);
    portDebugPrintf("RSSI %d", packet->rssi);
    portDebugPrintf("SNR %d", packet->snr);
    portDebugPrintf("Downlink Counter %lu", packet->counter);
}

// Assuming a continuous loop to process incoming data
while (1) {
    XBeeProcess(myXbeeLr);
    usleep(10000);  // Sleep for 10ms to avoid busy-waiting
}
```

### Closing the Connection

After completing your operations, disconnect the XBee LR module, clean up resources, and free the XBee LR instance:

```c
XBeeDisconnect(myXbeeLr);
XBeeClose(myXbeeLr);
XBeeDestroy(myXbeeLr);  // Free the XBee LR instance
printf("XBee LR module disconnected and resources cleaned up.");
```

### Full Example

Here is the full example code combining the steps mentioned above:

```c
#include "xbee_lr.h"
#include "port.h"

XBeeLR* myXbeeLr;

// Hardware Abstraction Function Pointer Table for XBeeLR
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
    .OnReceiveCallback = OnReceiveCallback, // Callback for receiving data
    .OnSendCallback = NULL,                 // Can be left as NULL if no callbacks are needed
};

void OnReceiveCallback(XBee* self, void* data){
    XBeeLRPacket_t* packet = (XBeeLRPacket_t*) data;
    portDebugPrintf("Received Packet: ");
    for (int i = 1; i < packet->payloadSize; i++) {
        portDebugPrintf("%02X ", packet->payload[i]);
    }
    portDebugPrintf("");
    portDebugPrintf("Ack %u", packet->ack);
    portDebugPrintf("Port %u", packet->port);
    portDebugPrintf("RSSI %d", packet->rssi);
    portDebugPrintf("SNR %d", packet->snr);
    portDebugPrintf("Downlink Counter %lu", packet->counter);
}

int main() {
    // Create the XBee LR instance with hardware and command tables
    myXbeeLr = XBeeLRCreate(&XBeeLRHTable, &XBeeLRCTable);
    if (myXbeeLr == NULL) {
        printf("Failed to create XBee LR instance.");
        return -1;
    }

    // Initialize the XBee LR module
    if (!XBeeLRInit(myXbeeLr, 9600, "COM3")) {  // Replace "COM3" with the appropriate serial port
        printf("Failed to initialize XBee LR module.");
        return -1;
    }

    // Read LoRaWAN DevEUI and print it
    uint8_t devEui[17];
    XBeeLRGetDevEUI((XBee*)myXbeeLr, devEui, sizeof(devEui));
    portDebugPrintf("DEVEUI: %s", devEui);

    // Set LoRaWAN Network Settings
    portDebugPrintf("Configuring...");
    XBeeLRSetAppEUI((XBee*)myXbeeLr, "37D56A3F6CDCF0A5");
    XBeeLRSetAppKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeLRSetNwkKey((XBee*)myXbeeLr, "CD32AAB41C54175E9060D86F3A8B7F48");
    XBeeWriteConfig((XBee*)myXbeeLr);
    XBeeApplyChanges((XBee*)myXbeeLr);

    // Connect to the LoRaWAN network
    portDebugPrintf("Connecting...");
    XBeeConnect((XBee*)myXbeeLr);

    uint8_t examplePayload[] = {0xC0, 0xC0, 0xC0, 0xFF, 0xEE};
    uint16_t payloadLen = sizeof(examplePayload) / sizeof(examplePayload[0]);
    XBeeLRPacket_t packet = {
        .payload = examplePayload,
        .payloadSize = payloadLen,
        .port = 2,
        .ack = 0,
    };

    if (!XBeeSendPacket(myXbeeLr, &packet)) {
        printf("Failed to send data.");
    } else {
        printf("Data sent successfully.");
    }

    // Main loop to process incoming data
    while (1) {
        XBeeProcess(myXbeeLr);
        usleep(10000);  // Sleep for 10ms
    }

    XBeeDisconnect(myXbeeLr);
    XBeeClose(myXbeeLr);
    XBeeDestroy(myXbeeLr);  // Free the XBee LR instance
    printf("XBee LR module disconnected and resources cleaned up.");

    return 0;
}
```


## How to Port to Other Platforms

Porting the XBee C library to other platforms involves adapting the code to the specific hardware and software environment of the new platform. The following steps outline the general process for porting the library:

### 1. Understand the Target Platform
Before starting the porting process, gather detailed information about the target platform, including:
- CPU architecture
- Operating system (if any)
- Available compilers and development tools
- Peripheral interfaces (UART, SPI, I2C, etc.)

### 2. Configure the Build Environment
Set up a development environment compatible with the target platform. This includes:
- Installing the appropriate compiler and toolchain
- Configuring makefiles or build scripts for the target platform

### 3. Modify Hardware Abstraction Layer (HAL)
The XBee library uses a HAL to interact with hardware peripherals. Modify the HAL implementation to match the target platform's peripherals:
- Update UART initialization and configuration
- Adjust GPIO settings if needed
- Implement any additional platform-specific peripheral control functions

### 4. Adjust Compiler and Linker Settings
Modify the compiler and linker settings to match the target platform:
- Set the correct optimization flags for the target CPU
- Adjust memory settings and linker scripts if required

### 5. Test and Debug
Thoroughly test the ported library on the target platform:
- Verify basic communication with the XBee module
- Test all major library functions
- Debug any issues that arise during testing

### 6. Optimize for Performance
After successful porting and testing, consider optimizing the library for performance on the target platform:
- Optimize critical sections of code for speed
- Reduce memory usage if necessary
- Take advantage of platform-specific features (e.g., DMA for UART transfers)

### 7. Document the Port
Finally, document any changes made during the porting process and provide instructions for others who may need to port the library to similar platforms.


## Adding a New Subclass for a New XBee Module

If you want to add support for a new XBee module, you will need to create a new subclass that extends the functionality of the base XBee class. This guide will walk you through the steps to add a new subclass and integrate it into the existing XBee framework.

### 1. Define the Subclass Structure

Start by creating a new header and source file for your subclass. Let's assume your new XBee module is called `XBeeNew`. Create `xbee_new.h` and `xbee_new.c`.

In `xbee_new.h`, define the subclass structure:

```c
#ifndef XBEE_NEW_H
#define XBEE_NEW_H

#include "xbee.h"

typedef struct {
    XBee base;  /**< Base class structure. */
    // Add any additional members specific to XBeeNew
} XBeeNew;

// Function prototypes for the subclass
XBeeNew* XBeeNewCreate(const XBeeCTable* cTable, const XBeeHTable* hTable);
void XBeeNewDestroy(XBeeNew* self);

#endif // XBEE_NEW_H
```

### 2. Implement the Subclass Methods

In `xbee_new.c`, implement the methods specific to your new XBee module. This will include the creation and destruction methods, as well as any specific behaviors your module requires.

```c
#include "xbee_new.h"
#include <stdlib.h>

static bool XBeeNewInit(XBee* self, uint32_t baudrate, const char* device);
static void XBeeNewProcess(XBee* self);
static bool XBeeNewConnect(XBee* self);
static bool XBeeNewDisconnect(XBee* self);
static bool XBeeNewSendData(XBee* self, const void* data);
static bool XBeeNewConnected(XBee* self);
static void XBeeNewHandleRxPacket(XBee* self, void* param);
static void XBeeNewHandleTransmitStatus(XBee* self, void* param);

// Define the VTable for the new XBee module
static const XBeeVTable XBeeNew_VTable = {
    .init = XBeeNewInit,
    .process = XBeeNewProcess,
    .connect = XBeeNew_Connect,
    .disconnect = XBeeewDisconnect,
    .sendData = XBeeNewSendData,
    .softReset = XBeeSoftReset,
    .hardReset = XBeeHardReset,
    .connected = XBeeNewConnected,
    .handleRxPacketFrame = XBeeNewHandleRxPacket,
    .handleTransmitStatusFrame = XBeeNewHandleTransmitStatus,
};

// Implement the create and destroy methods
XBeeNew* XBeeNewCreate(const XBeeCTable* cTable, const XBeeHTable* hTable) {
    XBeeNew* instance = (XBeeNew*)malloc(sizeof(XBeeNew));
    instance->base.vtable = &XBeeNew_VTable;
    instance->base.htable = hTable;
    instance->base.ctable = cTable;
    return instance;
}

void XBeeNewDestroy(XBeeNew* self) {
    free(self);
}

// Implement the specific methods for the new XBee module
static bool XBeeNewInit(XBee* self, uint32_t baudrate, const char* device) {
    // Implement initialization logic specific to XBeeNew
    return true;
}

static void XBeeNewProcess(XBee* self) {
    // Implement process logic specific to XBeeNew
}

static bool XBeeNewConnect(XBee* self) {
    // Implement connection logic specific to XBeeNew
    return true;
}

static bool XBeeNewDisconnect(XBee* self) {
    // Implement disconnection logic specific to XBeeNew
    return true;
}

static bool XBeeNewSendData(XBee* self, const void* data) {
    // Implement data sending logic specific to XBeeNew
    return true;
}

static bool XBeeNewConnected(XBee* self) {
    // Implement logic to check if XBeeNew is connected
    return true;
}

static void XBeeNewHandleRxPacket(XBee* self, void* param) {
    // Implement logic to handle received packets specific to XBeeNew
}

static void XBeeNewHandleTransmitStatus(XBee* self, void* param) {
    // Implement logic to handle transmit status specific to XBeeNew
}
```

### 3. Integrate the Subclass into Your Application

After defining and implementing your subclass, you can use it in your application by creating an instance of `XBeeNew` and using it like any other XBee module. See various example code.

### 4. Customize the Subclass as Needed

Depending on the specific features and requirements of your new XBee module, you may need to add or modify methods and behaviors in the subclass. The provided structure and functions offer a flexible framework that can be easily extended to support additional features.

## Contributing Guidelines

Thank you for considering contributing to our project! To ensure a smooth and consistent development process, please follow these guidelines when contributing:

### Coding Style

- **Google C++ Style Guide**: Our codebase follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html). Please ensure your contributions adhere to these guidelines.
  - **Naming Conventions**:
    - **Functions and Variables**: Use `lowerCamelCase` for function and variable names.
    - **Class and Struct Names**: Use `UpperCamelCase`.
    - **Macros**: Use `UPPER_SNAKE_CASE`.
    - **Enum Names**: Enum type names should use `UpperCamelCase`, and enum values should use `kUpperCamelCase`.
  - **File Names**:
    - All file names should be in lowercase with words separated by underscores (`_`).
    - Use `.h` for header files and `.c` for C implementation files.
  - **Indentation**: Use 2 spaces for indentation. Do not use tabs.

### Version of C

- **C Standard**: This project adheres to the C99 standard. Ensure your code is compatible with C99 to maintain consistency and compatibility across the codebase.
  - Use `stdint.h` for fixed-width integer types.
  - Avoid using non-standard extensions or compiler-specific features unless absolutely necessary.

### Considerations for Embedded Devices

When developing for embedded systems, it's important to be mindful of the resource constraints inherent in such environments:

- **Memory Usage**: Embedded devices typically have limited memory (both RAM and ROM). Avoid using large libraries or functions that require significant memory. For example:
  - **Avoid `sprintf`**: Instead of `sprintf`, consider using more memory-efficient alternatives like `snprintf` or custom formatting functions.
  - **Avoid Dynamic Memory Allocation**: Minimize or avoid the use of dynamic memory allocation (e.g., `malloc`, `free`) to prevent memory fragmentation and leaks.
- **Performance**: Optimize for low power consumption and execution speed. This includes minimizing CPU cycles, reducing the complexity of algorithms, and leveraging hardware-specific features where possible.
- **Code Size**: Keep the code size small to fit within the limited flash memory. This may involve stripping out unnecessary features or using compiler optimization flags specific to size reduction.


### Submitting Contributions

1. **Fork the Repository**: Start by forking the repository to your own GitHub account.
2. **Create a Branch**: Create a new branch for your feature or bug fix.
   - Use descriptive branch names (e.g., `feature/add-new-api` or `bugfix/fix-crash`).
3. **Make Your Changes**: Implement your changes in the codebase, ensuring they adhere to the coding style guidelines mentioned above.
4. **Test Your Changes**: Thoroughly test your code to ensure it functions correctly and does not introduce any regressions.
5. **Commit and Push**: Commit your changes with clear and concise commit messages, then push them to your forked repository.
6. **Create a Pull Request**: Open a pull request against the main repository's `main` branch. Include a detailed description of your changes and any relevant issue numbers.

### Testing and Documentation

- **Unit Tests**: If your changes introduce new functionality, please include corresponding unit tests to verify the behavior.
- **Documentation**: Update the documentation if your changes affect the public API or usage instructions.

### Code Review

- Your pull request will be reviewed by one or more maintainers. Please be responsive to feedback and make any necessary changes promptly.

### Licensing

- By contributing to this project, you agree that your contributions will be licensed under the project's existing license.

We appreciate your contributions and thank you for helping to improve this project!
