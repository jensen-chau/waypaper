#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "context.h"
#include "wayland_context.h"


int main(int argc, char *argv[]) {
    struct Context* context = get_context(1920, 1080); // 使用默认分辨率
    
    LOG("Initializing context...\n");

    if (!context) {
        fprintf(stderr, "Failed to initialize context\n");
        return 1;
    }

    struct WaylandContext* wayland_ctx = context->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Failed to get wayland context\n");
        return 1;
    }

    int loaded = 0;
    // 检查是否提供了壁纸路径参数
    if (argc >= 2) {
        LOG("Loading wallpaper from: %s\n", argv[1]);
        loaded = load_wallpaper(argv[1]);
    } else {
        // 使用默认壁纸路径
        loaded = load_wallpaper("/home/zjx/Pictures/wallpaper/【哲风壁纸】夜空-夜空繁星-山顶.jpg");
    }

    if (loaded == -1) {
        ERR("Failed to load wallpaper\n");
    }

    run();

    printf("Wayland context created successfully with %d outputs\n", wayland_ctx->num_outputs);

    return 0;
}
