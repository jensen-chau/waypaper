#!/bin/bash

# 测试壁纸切换功能

# 壁纸目录
WALLPAPER_DIR="/home/zjx/Pictures/wallpaper"

# 壁纸列表
WALLPAPERS=(
    "${WALLPAPER_DIR}/01.jpg"
    "${WALLPAPER_DIR}/02.png"
    "${WALLPAPER_DIR}/03.jpg"
    "${WALLPAPER_DIR}/04.png"
)

echo "====================================="
echo "开始测试壁纸切换功能"
echo "====================================="

# 检查 waypaper-daemon 是否在运行
if pgrep -f "waypaper-daemon" > /dev/null; then
    echo "waypaper-daemon 已在运行，先关闭它"
    ./build/waypaper-client shutdown
    sleep 1
fi

# 启动 waypaper-daemon
echo "启动 waypaper-daemon..."
./build/waypaper-daemon > /tmp/waypaper-daemon.log 2>&1 &
DAEMON_PID=$!
echo "waypaper-daemon PID: $DAEMON_PID"

# 等待 daemon 启动
sleep 2

echo ""
echo "====================================="
echo "测试切换壁纸功能"
echo "====================================="

# 循环切换壁纸
for i in {1..5}; do
    echo ""
    echo "[$i] 切换到壁纸: ${WALLPAPERS[$((i % 4))]}"
    ./build/waypaper-client set "${WALLPAPERS[$((i % 4))]}"

    # 等待一下
    sleep 2
done

echo ""
echo "====================================="
echo "测试完成"
echo "====================================="

# 显示 daemon 日志
echo ""
echo "Daemon 日志:"
echo "====================================="
tail -50 /tmp/waypaper-daemon.log

# 关闭 daemon
echo ""
echo "关闭 waypaper-daemon..."
./build/waypaper-client shutdown
wait $DAEMON_PID
echo "waypaper-daemon 已关闭"