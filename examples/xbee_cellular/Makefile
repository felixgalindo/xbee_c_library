# Example: examples/xbee_cellular/Makefile

# Platform selection
PLATFORM ?= unix

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR)

# Directories
SRC_DIR     = ../../src
INC_DIR     = ../../include
PORTS_DIR   = ../../ports
EXAMPLE_DIR = .
BUILD_DIR   = build/$(PLATFORM)

# Source files
CORE_SRCS = $(SRC_DIR)/xbee.c \
            $(SRC_DIR)/xbee_api_frames.c \
            $(SRC_DIR)/xbee_at_cmds.c \
            $(SRC_DIR)/xbee_cellular.c

PORT_SRC = $(PORTS_DIR)/port_$(PLATFORM).c

EXAMPLE_HTTP = $(EXAMPLE_DIR)/xbee_cellular_http_get_example.c
EXAMPLE_UDP  = $(EXAMPLE_DIR)/xbee_cellular_udp_echo_example.c

OBJS_HTTP = $(patsubst %.c, $(BUILD_DIR)/%.o, \
             $(notdir $(CORE_SRCS)) $(notdir $(EXAMPLE_HTTP)) $(notdir $(PORT_SRC)))

OBJS_UDP = $(patsubst %.c, $(BUILD_DIR)/%.o, \
            $(notdir $(CORE_SRCS)) $(notdir $(EXAMPLE_UDP)) $(notdir $(PORT_SRC)))

TARGET_HTTP = $(BUILD_DIR)/xbee_cellular_http_get_example
TARGET_UDP  = $(BUILD_DIR)/xbee_cellular_udp_echo_example

# Default rule
all: $(BUILD_DIR) $(TARGET_HTTP) $(TARGET_UDP)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Pattern rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(PORTS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linking
$(TARGET_HTTP): $(OBJS_HTTP)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_UDP): $(OBJS_UDP)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf build/$(PLATFORM)

.PHONY: all clean