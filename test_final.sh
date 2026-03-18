#!/bin/bash

# 最终测试：快速切换壁纸，检查是否能立即生效

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
./build/waypaper-daemon > /tmp/waypaper-final.log 2>&1 &
DAEMON_PID=$!
echo "waypaper-daemon PID: $DAEMON_PID"

# 等待 daemon 启动
sleep 2

echo ""
echo "====================================="
echo "快速切换壁纸测试（不等待）"
echo "====================================="

# 快速切换壁纸，不等待
echo "切换壁纸 1..."
./build/waypaper-client set "${WALLPAPER_DIR}/01.jpg"

echo "切换壁纸 2..."
./build/waypaper-client set "${WALLPAPER_DIR}/02.png"

echo "切换壁纸 3..."
./build/waypaper-client set "${WALLPAPER_DIR}/03.jpg"

echo "切换壁纸 4..."
./build/waypaper-client set "${WALLPAPER_DIR}/04.png"

echo "切换壁纸 5..."
./build/waypaper-client set "${WALLPAPER_DIR}/01.jpg"

echo ""
echo "等待 2 秒，让 daemon 处理所有请求..."
sleep 2

echo ""
echo "关闭 daemon..."
./build/waypaper-client shutdown
wait $DAEMON_PID

echo ""
echo "====================================="
echo "测试结果"
echo "====================================="

# 统计切换次数
SWITCH_COUNT=$(grep -c "Setting wallpaper" /tmp/waypaper-final.log)
echo "成功切换壁纸次数: $SWITCH_COUNT"

# 统计 buffer 释放次数
RELEASE_COUNT=$(grep -c "Buffer.*released by compositor" /tmp/waypaper-final.log)
echo "Buffer 释放次数: $RELEASE_COUNT"

# 统计 buffer 销毁次数
DESTROY_COUNT=$(grep -c "Current buffer.*released, destroying it" /tmp/waypaper-final.log)
echo "Buffer 销毁次数: $DESTROY_COUNT"

# 检查是否有错误
ERROR_COUNT=$(grep -c "ERR\|Failed\|Error" /tmp/waypaper-final.log)
echo "错误次数: $ERROR_COUNT"

if [ $ERROR_COUNT -eq 0 ]; then
    echo ""
    echo "✓ 测试通过：没有发现错误"
else
    echo ""
    echo "✗ 测试失败：发现 $ERROR_COUNT 个错误"
    echo ""
    echo "错误详情:"
    grep -E "ERR|Failed|Error" /tmp/waypaper-final.log
fi

echo ""
echo "时间线分析:"
echo "====================================="
grep "Setting wallpaper\|Buffer.*released" /tmp/waypaper-final.log | head -20