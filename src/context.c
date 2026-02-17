#include "context.h"

#include <stdlib.h>
#include <unistd.h>

#include "wayland_context.h"

struct Node;

static int should_exit = 0;
static struct Context* ctx;

struct Context* get_context(int width, int height) {
    if (ctx != NULL) {
        return ctx;
    }
    ctx = malloc(sizeof(struct Context));
    ctx->wayland_context = wayland_context_init(width, height);
    return ctx;
}

void app_exit() {
    should_exit = 1;
}

void run(struct Node* root) {
    struct WaylandContext* wayland_ctx = ctx->wayland_context;

    int color = 0x000000FF;
    int begin = 0;
    int delta = 1;

    while (!should_exit) {
        // 检查缓冲区是否有效

        root->node_draw(root);

        /*
        if (!wayland_ctx->buffer) {
            fprintf(stderr, "Error: buffer is NULL\n");
            break;
        }


        // 填充红色背景
        for (int i = 0; i < 200 * 200; i++) {
            for (int j = 0; j < 200; j++) {
                draw_point(i, j, color);
            }
        }

        color++;

        // 绘制移动的黑色方块
        for (int i = begin; i < 10 + begin && i < 200; i++) {
            for (int j = 95; j < 105 && j < 200; j++) {
                draw_point(i, j, 0x00000000);
            }
        }*/

        wl_surface_attach(wayland_ctx->surface, wayland_ctx->buffer, 0, 0);
        wl_surface_damage(wayland_ctx->surface, 0, 0, 200, 200);
        wl_surface_commit(wayland_ctx->surface);

        // 处理待处理的 Wayland 事件
        wl_display_flush(wayland_ctx->display);
        while (wl_display_prepare_read(wayland_ctx->display) != 0) {
            wl_display_dispatch_pending(wayland_ctx->display);
        }
        wl_display_read_events(wayland_ctx->display);
        wl_display_dispatch_pending(wayland_ctx->display);

        /*begin += delta;

        if (begin >= 190) {
            delta = -1;
        } else if (begin == 0) {
            delta = 1;
        }*/

        usleep(16667);
    }

    wayland_context_cleanup(wayland_ctx);
    free(ctx);
}

void draw_point(int x, int y, uint32_t color) {
    ((uint32_t*)
         ctx->wayland_context->shm_data)[x + y * ctx->wayland_context->width] =
        color;
}
