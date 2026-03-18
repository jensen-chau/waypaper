#!/bin/bash

# 简单的壁纸切换测试

# 壁纸目录
WALLPAPER_DIR="/home/zjx/Pictures/wallpaper"

# 检查 waypaper-daemon 是否在运行
if pgrep -f "waypaper-daemon" > /dev/null; then
    echo "waypaper-daemon 已在运行，先关闭它"
    ./build/waypaper-client shutdown
    sleep 2
fi

# 启动 waypaper-daemon
echo "启动 waypaper-daemon..."
./build/waypaper-daemon > /tmp/waypaper-daemon-simple.log 2>&1 &
DAEMON_PID=$!
echo "waypaper-daemon PID: $DAEMON_PID"

# 等待 daemon 启动
sleep 2

echo ""
echo "切换壁纸 1..."
./build/waypaper-client set "${WALLPAPER_DIR}/01.jpg"
sleep 1

echo ""
echo "切换壁纸 2..."
./build/waypaper-client set "${WALLPAPER_DIR}/02.png"
sleep 1

echo ""
echo "切换壁纸 3..."
./build/waypaper-client set "${WALLPAPER_DIR}/03.jpg"
sleep 1

echo ""
echo "切换壁纸 4..."
./build/waypaper-client set "${WALLPAPER_DIR}/04.png"
sleep 1

echo ""
echo "测试完成，关闭 daemon..."
./build/waypaper-client shutdown
wait $DAEMON_PID

echo ""
echo "Daemon 日志:"
echo "====================================="
tail -30 /tmp/waypaper-daemon-simple.log

echo ""
echo "Buffer 释放日志:"
echo "====================================="
grep "Buffer.*released" /tmp/waypaper-daemon-simple.log

echo ""
echo "Old buffer 销毁日志:"
echo "====================================="
grep "Old buffer" /tmp/waypaper-daemon-simple.log || echo "没有找到 old buffer 销毁日志"

echo ""
echo "Current buffer 销毁日志:"
echo "====================================="
grep "Current buffer" /tmp/waypaper-daemon-simple.log || echo "没有找到 current buffer 销毁日志"