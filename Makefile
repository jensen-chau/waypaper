CC = gcc

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

# 共享的核心源文件
CORE_SOURCES = src/wayland_context.c \
               src/context.c \
               src/utils.c \
               src/scale.c \
			   src/ipc.c

CORE_OBJECTS = $(CORE_SOURCES:src/%.c=$(BUILD_DIR)/core-%.o)

# 主程序源文件
MAIN_SOURCES = src/main.c
MAIN_OBJECTS = $(MAIN_SOURCES:src/%.c=$(BUILD_DIR)/main-%.o)

# 客户端源文件
CLIENT_SOURCES = src/client/client.c
CLIENT_OBJECTS = $(CLIENT_SOURCES:src/client/%.c=$(BUILD_DIR)/client-%.o)

# 守护进程源文件
DAEMON_SOURCES = src/daemon/daemon.c
DAEMON_OBJECTS = $(DAEMON_SOURCES:src/daemon/%.c=$(BUILD_DIR)/daemon-%.o)

# 协议对象文件
PROTOCOL_OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(PROTOCOL_SOURCES:.c=.o)))

# 库
WAYLAND_LIBS = -lwayland-client
MATH_LIBS = -lm
XBKB_LIBS = -lxkbcommon

# 链接标志
MAIN_LDFLAGS = $(WAYLAND_LIBS) $(MATH_LIBS) $(XBKB_LIBS) -lpthread
CLIENT_LDFLAGS = -lpthread
DAEMON_LDFLAGS = $(WAYLAND_LIBS) $(MATH_LIBS) $(XBKB_LIBS) -lpthread

# 编译标志
CFLAGS = -g -Wall $(INCLUDES) -I$(INCLUDE_PROTOCOL_DIR) -DSTB_IMAGE_RESIZE_IMPLEMENTATION

# 目标名称
MAIN_TARGET = waypaper
CLIENT_TARGET = waypaper-client
DAEMON_TARGET = waypaper-daemon

.PHONY: all clean run main client daemon

default: all

all: $(BUILD_DIR)/$(MAIN_TARGET) $(BUILD_DIR)/$(CLIENT_TARGET) $(BUILD_DIR)/$(DAEMON_TARGET)

# 构建主程序
main: $(BUILD_DIR)/$(MAIN_TARGET)

$(BUILD_DIR)/$(MAIN_TARGET): $(MAIN_OBJECTS) $(CORE_OBJECTS) $(PROTOCOL_OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(MAIN_LDFLAGS)

# 构建客户端
client: $(BUILD_DIR)/$(CLIENT_TARGET)

$(BUILD_DIR)/$(CLIENT_TARGET): $(CLIENT_OBJECTS) $(BUILD_DIR)/core-ipc.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(CLIENT_LDFLAGS)

# 构建守护进程
daemon: $(BUILD_DIR)/$(DAEMON_TARGET)

$(BUILD_DIR)/$(DAEMON_TARGET): $(DAEMON_OBJECTS) $(CORE_OBJECTS) $(PROTOCOL_OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(DAEMON_LDFLAGS)

# 核心对象文件规则（共享）
$(BUILD_DIR)/core-%.o: src/%.c $(PROTOCOL_HEADERS) $(INCLUDE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 主程序对象文件规则
$(BUILD_DIR)/main-%.o: src/%.c $(PROTOCOL_HEADERS) $(INCLUDE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 客户端对象文件规则
$(BUILD_DIR)/client-%.o: src/client/%.c $(INCLUDE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 守护进程对象文件规则
$(BUILD_DIR)/daemon-%.o: src/daemon/%.c $(INCLUDE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 协议对象文件规则
$(BUILD_DIR)/%-protocol.o: $(SRC_PROTOCOL_DIR)/%-protocol.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 协议头文件生成
$(INCLUDE_PROTOCOL_DIR)/%-protocol.h: $(PROTOCOL_DIR)/%.xml
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $< $@

# 协议源文件生成
$(SRC_PROTOCOL_DIR)/%-protocol.c: $(PROTOCOL_DIR)/%.xml
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $< $@

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $@

# 清理
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(PROTOCOL_SOURCES) $(PROTOCOL_HEADERS)

# 运行主程序
run: all
	@./$(BUILD_DIR)/$(MAIN_TARGET)
