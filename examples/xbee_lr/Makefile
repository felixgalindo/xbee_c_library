# Example: examples/xbee_lr/Makefile

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
            $(SRC_DIR)/xbee_lr.c

EXAMPLE_SRC = $(EXAMPLE_DIR)/xbee_lr_example.c
PORT_SRC    = $(PORTS_DIR)/port_$(PLATFORM).c

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, \
        $(notdir $(CORE_SRCS)) $(notdir $(EXAMPLE_SRC)) $(notdir $(PORT_SRC)))

# Output binary
TARGET = $(BUILD_DIR)/xbee_lr_example

# Default rule
all: $(BUILD_DIR) $(TARGET)

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
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf build/$(PLATFORM)

.PHONY: all clean