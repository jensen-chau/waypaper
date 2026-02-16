#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "wayland_context.h"

static volatile int should_exit = 0;

void key_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
            uint32_t time, uint32_t key, uint32_t state) {
    printf("key_handler: serial=%u, time=%u, key=%u, state=%s\n", 
           serial, time, key, state == WL_KEYBOARD_KEY_STATE_PRESSED ? "PRESSED" : "RELEASED");
    
    // 如果是按键按下事件，可以在这里添加处理逻辑
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        // 例如：检测 ESC 键退出
        if (key == 9) { // ESC 键的键码
            printf("ESC key pressed, exiting...\n");
            should_exit = 1;
        }
    }
}

int main() {
    printf("Hello, World!\n");

    struct WaylandContext* ctx = wayland_context_init(200, 200);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize Wayland context\n");
        return 1;
    }

    printf("Wayland context created successfully\n");
    
    int begin = 0;
    int delta = 1;

    uint32_t color = 0x00000000;

    while (!should_exit) {
        // 检查缓冲区是否有效
        if (!ctx->buffer) {
            fprintf(stderr, "Error: buffer is NULL\n");
            break;
        }
        
        // 填充红色背景
        for (int i = 0; i < 200 * 200; i++) {
            ((uint32_t*)ctx->shm_data)[i] = color;
        }

        color++;

        // 绘制移动的黑色方块
        for (int i=begin; i<10+begin && i<200; i++) {
            for (int j=95; j<105 && j<200; j++) {
                ((uint32_t*)ctx->shm_data)[j*200 + i] = 0x00000000;
            }
        }
        
        wl_surface_attach(ctx->surface, ctx->buffer, 0, 0);
        wl_surface_damage(ctx->surface, 0, 0, 200, 200);
        wl_surface_commit(ctx->surface);
        
        // 处理待处理的 Wayland 事件
        wl_display_dispatch(ctx->display);

        begin += delta;

        if (begin >= 190) {
            delta = -1;
        } else if(begin == 0) {
            delta = 1;
        }

        usleep(16667);
    }
    
    wayland_context_cleanup(ctx);
    return 0;
}
