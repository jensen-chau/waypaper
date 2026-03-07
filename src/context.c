#define STB_IMAGE_IMPLEMENTATION
#include "context.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "wayland_context.h"

struct Node;

static int should_exit = 0;
static struct Context* ctx = NULL;

struct Context* get_context(int width, int height) {
    if (ctx != NULL) {
        return ctx;
    }
    ctx = malloc(sizeof(struct Context));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate context\n");
        return NULL;
    }

    ctx->wayland_context = wayland_context_init(width, height);
    if (!ctx->wayland_context) {
        fprintf(stderr, "Failed to initialize wayland context\n");
        free(ctx);
        ctx = NULL;
        return NULL;
    }
    return ctx;
}

void app_exit() {
    should_exit = 1;
}

void load_wallpaper(const char* path) {
    int width, height;
    int channels_in_file;
    unsigned char* img_data =
        stbi_load(path, &width, &height, &channels_in_file, 3);
    if (img_data == NULL) {
        fprintf(stderr, "Failed to load image\n");
        return;
    }
    printf("Image loaded: %dx%d, channels:%d\n", width, height,
           channels_in_file);

    create_pool(ctx->wayland_context, width, height, channels_in_file);

    ctx->wayland_context->shm_data = (void*)img_data;
}

void run() {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return;
    }

    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Error: wayland context is NULL\n");
        return;
    }

    uint32_t cnt = 0;

    load_wallpaper("/home/zjx/Pictures/wallpaper/01.jpg");

    while (!should_exit) {
        if (++cnt % 60 == 0) {
            printf("frame: %d\n", cnt);
        }

        // 检查缓冲区是否有效
        if (!wayland_ctx->buffer) {
            fprintf(stderr, "Error: buffer is NULL\n");
            break;
        }

        wl_surface_attach(wayland_ctx->surface, wayland_ctx->buffer, 0, 0);
        wl_surface_damage(wayland_ctx->surface, 0, 0, wayland_ctx->width,
                          wayland_ctx->height);
        wl_surface_commit(wayland_ctx->surface);

        // 处理待处理的 Wayland 事件
        wl_display_flush(wayland_ctx->display);
        while (wl_display_prepare_read(wayland_ctx->display) != 0) {
            wl_display_dispatch_pending(wayland_ctx->display);
        }
        wl_display_read_events(wayland_ctx->display);
        wl_display_dispatch_pending(wayland_ctx->display);

        usleep(16667);
    }

    wayland_context_cleanup(wayland_ctx);
    free(ctx);
    ctx = NULL;
}

void handle_event(PointEvent point_event, void* data) {
    printf("point_event:%d\n", point_event);
}

Point get_mouse_point_pos() {
    return ctx->mouse_pos;
}
