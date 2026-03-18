#!/bin/bash

# 直接测试 main 程序（不使用 daemon）

# 壁纸目录
WALLPAPER_DIR="/home/zjx/Pictures/wallpaper"

echo "====================================="
echo "直接测试壁纸切换功能"
echo "====================================="

# 检查是否有其他 waypaper 进程在运行
if pgrep -f "waypaper" > /dev/null; then
    echo "发现其他 waypaper 进程，先关闭它们"
    pkill -9 -f "waypaper"
    sleep 1
fi

echo ""
echo "测试 1: 加载壁纸 01.jpg"
echo "====================================="
timeout 3 ./build/waypaper "${WALLPAPER_DIR}/01.jpg" 2>&1 | tee /tmp/waypaper-test-1.log || true
sleep 1

echo ""
echo "测试 2: 加载壁纸 02.png"
echo "====================================="
timeout 3 ./build/waypaper "${WALLPAPER_DIR}/02.png" 2>&1 | tee /tmp/waypaper-test-2.log || true
sleep 1

echo ""
echo "测试 3: 加载壁纸 03.jpg"
echo "====================================="
timeout 3 ./build/waypaper "${WALLPAPER_DIR}/03.jpg" 2>&1 | tee /tmp/waypaper-test-3.log || true
sleep 1

echo ""
echo "测试 4: 加载壁纸 04.png"
echo "====================================="
timeout 3 ./build/waypaper "${WALLPAPER_DIR}/04.png" 2>&1 | tee /tmp/waypaper-test-4.log || true
sleep 1

echo ""
echo "====================================="
echo "测试完成"
echo "====================================="

# 清理进程
pkill -9 -f "waypaper" 2>/dev/null || true

echo ""
echo "Buffer 释放日志汇总:"
echo "====================================="
echo "测试 1:"
grep "Buffer.*released" /tmp/waypaper-test-1.log || echo "  没有找到 buffer 释放日志"

echo ""
echo "测试 2:"
grep "Buffer.*released" /tmp/waypaper-test-2.log || echo "  没有找到 buffer 释放日志"

echo ""
echo "测试 3:"
grep "Buffer.*released" /tmp/waypaper-test-3.log || echo "  没有找到 buffer 释放日志"

echo ""
echo "测试 4:"
grep "Buffer.*released" /tmp/waypaper-test-4.log || echo "  没有找到 buffer 释放日志"