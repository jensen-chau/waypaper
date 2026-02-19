#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "box.h"
#include "context.h"
#include "node.h"
#include "button.h"
#include "label.h"
#include "text_input.h"
#include "checkbox.h"
#include "slider.h"
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

    // 创建标签
    struct Label* label = label_new("label1", 10, 10, "Hello, World!");
    if (label) {
        label_set_text_color(label, 0xFFFFFFFF); // 白色文本
        add_child(&box->node, &label->node);
    }

    // 创建水平布局的box
    struct Box* hbox = box_new("hbox1", Horizontal, 10, 100, 180, 60);
    if (hbox) {
        set_bg_color(&hbox->node, 0xFF333333); // 深灰色背景
        
        // 在水平box中添加一些控件
        struct Button* button2 = button_new("button2", 0, 0, 50, 30, "Btn1");
        if (button2) {
            set_bg_color(&button2->node, 0xFF6666FF);
            button_set_on_click(button2, on_button_click, NULL);
            add_child(&hbox->node, &button2->node);
        }
        
        struct Button* button3 = button_new("button3", 0, 0, 50, 30, "Btn2");
        if (button3) {
            set_bg_color(&button3->node, 0xFF6666FF);
            button_set_on_click(button3, on_button_click, NULL);
            add_child(&hbox->node, &button3->node);
        }
        
        struct Button* button4 = button_new("button4", 0, 0, 50, 30, "Btn3");
        if (button4) {
            set_bg_color(&button4->node, 0xFF6666FF);
            button_set_on_click(button4, on_button_click, NULL);
            add_child(&hbox->node, &button4->node);
        }
        
        add_child(&box->node, &hbox->node);
    }

    // 创建文本输入框
    struct TextInput* text_input = text_input_new("text_input1", 10, 170, 180, 30, "输入文本...");
    if (text_input) {
        text_input_set_text_color(text_input, 0xFFFFFFFF); // 白色文本
        text_input_set_background_color(text_input, 0xFF333333); // 深灰色背景
        text_input_set_border_color(text_input, 0xFF666666); // 灰色边框
        add_child(&box->node, &text_input->node);
    }

    // 创建垂直布局的box，用于放置更多控件
    struct Box* vbox = box_new("vbox1", Vertical, 10, 210, 180, 150);
    if (vbox) {
        set_bg_color(&vbox->node, 0xFF333333); // 深灰色背景
        
        // 添加复选框
        struct CheckBox* checkbox1 = checkbox_new("checkbox1", 0, 0, "启用通知");
        if (checkbox1) {
            checkbox_set_checked(checkbox1, 1); // 默认选中
            add_child(&vbox->node, &checkbox1->node);
        }
        
        struct CheckBox* checkbox2 = checkbox_new("checkbox2", 0, 0, "自动保存");
        if (checkbox2) {
            add_child(&vbox->node, &checkbox2->node);
        }
        
        // 添加滑块
        struct Slider* slider1 = slider_new("slider1", 0, 0, 160, 20, 0.0, 100.0, 50.0);
        if (slider1) {
            slider_set_track_color(slider1, 0xFF555555);
            slider_set_fill_color(slider1, 0xFF6666FF);
            slider_set_thumb_color(slider1, 0xFFFFFFFF);
            add_child(&vbox->node, &slider1->node);
        }
        
        // 添加垂直滑块
        struct Slider* slider2 = slider_new("slider2", 0, 0, 20, 120, 0.0, 100.0, 75.0);
        if (slider2) {
            slider_set_orientation(slider2, 1); // 垂直
            slider_set_track_color(slider2, 0xFF555555);
            slider_set_fill_color(slider2, 0xFF6666FF);
            slider_set_thumb_color(slider2, 0xFFFFFFFF);
            add_child(&vbox->node, &slider2->node);
        }
        
        add_child(&box->node, &vbox->node);
    }

    // 创建网格布局的box
    struct Box* grid_box = box_new("grid_box1", Grid, 210, 10, 180, 150);
    if (grid_box) {
        set_bg_color(&grid_box->node, 0xFF333333); // 深灰色背景
        
        // 添加按钮到网格
        for (int i = 0; i < 6; i++) {
            struct Button* button = button_new("grid_button", 0, 0, 0, 0, "");
            char btn_text[10];
            sprintf(btn_text, "Btn%d", i+1);
            button_set_text(button, btn_text);
            set_bg_color(&button->node, 0xFF6666FF);
            button_set_on_click(button, on_button_click, NULL);
            add_child(&grid_box->node, &button->node);
        }
        
        add_child(&box->node, &grid_box->node);
    }

    // 创建堆叠布局的box
    struct Box* stack_box = box_new("stack_box1", Stack, 210, 170, 180, 80);
    if (stack_box) {
        set_bg_color(&stack_box->node, 0xFF333333); // 深灰色背景
        
        // 添加背景标签
        struct Label* stack_label = label_new("stack_label", 0, 0, "堆叠布局示例");
        if (stack_label) {
            label_set_text_color(stack_label, 0xFFFFFFFF);
            add_child(&stack_box->node, &stack_label->node);
        }
        
        // 添加按钮
        struct Button* stack_button = button_new("stack_button", 0, 0, 60, 30, "叠加");
        if (stack_button) {
            button_set_text_color(stack_button, 0xFFFFFFFF);
            set_bg_color(&stack_button->node, 0xFF6666FF);
            add_child(&stack_box->node, &stack_button->node);
        }
        
        add_child(&box->node, &stack_box->node);
    }

    printf("Starting main loop...\n");
    run(&box->node);

    return 0;
}
