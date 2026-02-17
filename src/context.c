#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void run(struct Node* root) {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return;
    }
    
    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Error: wayland context is NULL\n");
        return;
    }
    
    if (!root) {
        fprintf(stderr, "Error: root node is NULL\n");
        return;
    }

    while (!should_exit) {
        // 检查缓冲区是否有效
        if (!wayland_ctx->buffer) {
            fprintf(stderr, "Error: buffer is NULL\n");
            break;
        }

        root->node_draw(root);

        wl_surface_attach(wayland_ctx->surface, wayland_ctx->buffer, 0, 0);
        wl_surface_damage(wayland_ctx->surface, 0, 0, wayland_ctx->width, wayland_ctx->height);
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

void draw_point(int x, int y, uint32_t color) {
    if (!ctx || !ctx->wayland_context || !ctx->wayland_context->shm_data) {
        fprintf(stderr, "Error: Invalid context in draw_point\n");
        return;
    }
    
    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (x < 0 || x >= wayland_ctx->width || y < 0 || y >= wayland_ctx->height) {
        return; // 边界检查
    }
    
    ((uint32_t*)wayland_ctx->shm_data)[x + y * wayland_ctx->width] = color;
}


void handle_event(PointEvent point_event, void *data) {
    printf("point_event:%d\n", point_event);
}
