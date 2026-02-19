#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "box.h"
#include "context.h"
#include "node.h"
#include "button.h"
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

// 按钮点击事件处理函数示例
void on_button_click(struct Button* button) {
    printf("Button '%s' clicked!\n", button->text);
    // 这里可以添加按钮点击后的处理逻辑
    struct Button* btn = (struct Button*)get_node("button1");
    button_set_text(btn, "hello world");
}

int main() {
    printf("Initializing context...\n");
    struct Context* context = get_context(200, 200);

    if (!context) {
        fprintf(stderr, "Failed to initialize context\n");
        return 1;
    }

    struct WaylandContext* wayland_ctx = context->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Failed to get wayland context\n");
        return 1;
    }

    printf("Wayland context created successfully\n");

    struct Box* box = box_new("box1", Vertical, 0, 0, 200, 200);
    if (!box) {
        fprintf(stderr, "Failed to create box\n");
        return 1;
    }

    set_bg_color(&box->node, 0xFF0000FF);

    struct Box* child1 = box_new("box2", Vertical, 0, 0, 100, 100);

    set_bg_color(&child1->node, 0x00FF00FF);
    add_child(&box->node, &child1->node);

    struct Box* child2 = box_new("box3", Vertical, 0, 0, 50, 50);
    set_bg_color(&child2->node, 0x0000FFFF);
    add_child(&child1->node, &child2->node);

    
    // 创建按钮
    struct Button* button = button_new("button1", 50, 50, 100, 40, "Click Me");
    if (button) {
        set_bg_color(&button->node, 0xFF6666FF);
        // 设置点击事件处理函数
        button_set_on_click(button, on_button_click, NULL);
        add_child(&box->node, &button->node);
    }

    printf("Starting main loop...\n");
    run(&box->node);

    return 0;
}
