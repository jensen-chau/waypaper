CC = gcc
TARGET = way_ui

INCLUDE = -Iinclude
SOURCES = src/main.c \
		  src/wayland_context.c
	
LIBS = -lwayland-client

BUILD_DIR = build

PROTOCOL_DIR = protocol
SRC_PROTOCOL_DIR = src/protocol
INCLUDE_PROTOCOL_DIR = include/protocol 

WAYLAND_SCANNER = wayland-scanner 

PROTOCOL_XMLS = $(wildcard $(PROTOCOL_DIR)/*.xml)
PROTOCOL_HEADERS = $(addprefix $(INCLUDE_PROTOCOL_DIR)/, $(notdir $(PROTOCOL_XMLS:.xml=-protocol.h)))
PROTOCOL_SOURCES = $(addprefix $(SRC_PROTOCOL_DIR)/, $(notdir $(PROTOCOL_XMLS:.xml=-protocol.c)))

OBJECTS = $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)

PROTOCOL_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(PROTOCOL_SOURCES:.c=.o)))

CFLAGS = -g -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable $(INCLUDE) -I$(INCLUDE_PROTOCOL_DIR)


.PHONY: all clean run

all: $(BUILD_DIR) $(INCLUDE_PROTOCOL_DIR) $(SRC_PROTOCOL_DIR) $(PROTOCOL_SOURCES) $(PROTOCOL_HEADERS) $(TARGET)


$(INCLUDE_PROTOCOL_DIR)/%-protocol.h: $(PROTOCOL_DIR)/%.xml | $(INCLUDE_PROTOCOL_DIR)
	$(WAYLAND_SCANNER) client-header $< $@

$(SRC_PROTOCOL_DIR)/%-protocol.c: $(PROTOCOL_DIR)/%.xml | $(SRC_PROTOCOL_DIR)
	$(WAYLAND_SCANNER) private-code $< $@

$(BUILD_DIR)/%-protocol.o: $(SRC_PROTOCOL_DIR)/%-protocol.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(OBJECTS) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

$(INCLUDE_PROTOCOL_DIR):
	@mkdir -p $(INCLUDE_PROTOCOL_DIR)

$(SRC_PROTOCOL_DIR):
	@mkdir -p $(SRC_PROTOCOL_DIR)

clean:
	@rm -r $(BUILD_DIR)

run: 
	@./$(BUILD_DIR)/$(TARGET)
