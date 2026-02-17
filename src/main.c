#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "box.h"
#include "context.h"
#include "node.h"
#include "wayland_context.h"


void key_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
            uint32_t time, uint32_t key, uint32_t state) {
    printf("key_handler: serial=%u, time=%u, key=%u, state=%s\n", 
           serial, time, key, state == WL_KEYBOARD_KEY_STATE_PRESSED ? "PRESSED" : "RELEASED");
    
    // 如果是按键按下事件，可以在这里添加处理逻辑
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        // 例如：检测 ESC 键退出
        if (key == 1) { // ESC 键的键码
            printf("ESC key pressed, exiting...\n");
            app_exit();
        }
    }
}

int main() {

    struct Context* context = get_context(200, 200);

    struct WaylandContext* ctx = context->wayland_context;

    if (!ctx) {
        fprintf(stderr, "Failed to initialize Wayland context\n");
        return 1;
    }

    printf("Wayland context created successfully\n");

    struct Box* box = box_new(Vertical, 0, 0, 200, 200);

    set_bg_color(&box->node, 0xFF0000FF);

    run(&box->node);

    return 0;
}
