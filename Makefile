CC = gcc
TARGET = way_ui

BUILD_DIR = build

INCLUDES = -Iinclude
PROTOCOL_DIR = protocol
SRC_PROTOCOL_DIR = src/protocol
INCLUDE_PROTOCOL_DIR = include/protocol


INCLUDE_HEADERS = $(wildcard include/*.h)
WAYLAND_SCANNER ?= wayland-scanner
PROTOCOL_XMLS = $(wildcard $(PROTOCOL_DIR)/*.xml)
PROTOCOL_HEADERS = $(addprefix $(INCLUDE_PROTOCOL_DIR)/, $(notdir $(PROTOCOL_XMLS:.xml=-protocol.h)))
PROTOCOL_SOURCES = $(addprefix $(SRC_PROTOCOL_DIR)/, $(notdir $(PROTOCOL_XMLS:.xml=-protocol.c)))

SOURCES = src/main.c \
          src/wayland_context.c \
		  src/context.c \
		  src/node.c \
		  src/box.c \
		  src/button.c \
		  src/label.c \
		  src/text_input.c \
		  src/checkbox.c \
		  src/slider.c \
		  src/utils.c

OBJECTS = $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
PROTOCOL_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(PROTOCOL_SOURCES:.c=.o)))

WAYLAND_LIBS = -lwayland-client
MATH_LIBS = -lm
XBKB_LIBS = -lxkbcommon
LDFLAGS = $(WAYLAND_LIBS) $(MATH_LIBS) $(XBKB_LIBS)

CFLAGS = -g -Wall $(INCLUDES) -I$(INCLUDE_PROTOCOL_DIR) $(shell pkg-config --cflags cairo pango pangocairo)
LIBS = $(shell pkg-config --libs cairo pango pangocairo)

CFLAGS += $(LIBS)

.PHONY: all clean run

default: all

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS) $(PROTOCOL_OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c $(PROTOCOL_HEADERS) $(INCLUDE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%-protocol.o: $(SRC_PROTOCOL_DIR)/%-protocol.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(INCLUDE_PROTOCOL_DIR)/%-protocol.h: $(PROTOCOL_DIR)/%.xml
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $< $@

$(SRC_PROTOCOL_DIR)/%-protocol.c: $(PROTOCOL_DIR)/%.xml
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $< $@

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(PROTOCOL_SOURCES) $(PROTOCOL_HEADERS)

run: all
	@./$(BUILD_DIR)/$(TARGET)
