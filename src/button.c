#include "button.h"
#include "context.h"
#include "wayland_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <glib.h>

struct Button* button_new(const char* id, int x, int y, int width, int height, const char* text) {
    struct Button* button = (struct Button*)malloc(sizeof(struct Button));
    if (!button) {
        fprintf(stderr, "Failed to allocate button\n");
        return NULL;
    }
    
    // 初始化node
    node_init(&button->node, id, x, y, width, height, Button);
    
    // 初始化按钮属性
    button->text = text ? strdup(text) : strdup("");
    button->text_color = 0xFFFFFFFF; // 白色
    button->border_color = 0xFF000000; // 黑色
    button->border_width = 1;
    
    // 创建Pango布局
    button->layout = NULL;
    
    button->node.node_draw = draw_button;
    button->node.is_hoverd = is_hoverd;
    
    return button;
}

void draw_button(struct Node* self) {
    if (!self || self->node_type != Button) {
        return;
    }
    
    struct Button* button = (struct Button*)self;
    struct Context* context = get_context(0, 0);
    if (!context) {
        // 回退到简单的矩形绘制
        for (int i = 0; i < self->width; i++) {
            for (int j = 0; j < self->height; j++) {
                draw_point(self->x + i, self->y + j, self->bg_color);
            }
        }
        return;
    }
    
    struct WaylandContext* wayland_ctx = context->wayland_context;
    
    if (!wayland_ctx || !wayland_ctx->cairo_context) {
        // 回退到简单的矩形绘制
        for (int i = 0; i < self->width; i++) {
            for (int j = 0; j < self->height; j++) {
                draw_point(self->x + i, self->y + j, self->bg_color);
            }
        }
        return;
    }
    
    // 检查鼠标是否在按钮上
    Point mouse_pos = get_mouse_point_pos();
    int is_hovered = (mouse_pos.x >= self->x && mouse_pos.x < self->x + self->width &&
                      mouse_pos.y >= self->y && mouse_pos.y < self->y + self->height);
    
    // 根据悬停状态调整背景颜色
    uint32_t draw_color = self->bg_color;
    if (is_hovered) {
        // 悬停时稍微变亮
        uint8_t r = (draw_color >> 16) & 0xFF;
        uint8_t g = (draw_color >> 8) & 0xFF;
        uint8_t b = draw_color & 0xFF;
        uint8_t a = (draw_color >> 24) & 0xFF;
        
        // 增加亮度
        r = (r * 120) / 100;
        g = (g * 120) / 100;
        b = (b * 120) / 100;
        
        draw_color = (a << 24) | (r << 16) | (g << 8) | b;
    }
    
    // 绘制按钮背景
    uint8_t bg_r = (draw_color >> 16) & 0xFF;
    uint8_t bg_g = (draw_color >> 8) & 0xFF;
    uint8_t bg_b = draw_color & 0xFF;
    uint8_t bg_a = (draw_color >> 24) & 0xFF;
    
    cairo_save(wayland_ctx->cairo_context);
    cairo_set_source_rgba(wayland_ctx->cairo_context, bg_r/255.0, bg_g/255.0, bg_b/255.0, bg_a/255.0);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_fill(wayland_ctx->cairo_context);
    
    // 绘制边框
    uint8_t border_r = (button->border_color >> 16) & 0xFF;
    uint8_t border_g = (button->border_color >> 8) & 0xFF;
    uint8_t border_b = button->border_color & 0xFF;
    uint8_t border_a = (button->border_color >> 24) & 0xFF;
    
    cairo_set_source_rgba(wayland_ctx->cairo_context, border_r/255.0, border_g/255.0, border_b/255.0, border_a/255.0);
    cairo_set_line_width(wayland_ctx->cairo_context, button->border_width);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_stroke(wayland_ctx->cairo_context);
    
    // 绘制文本
    if (button->text && button->text[0] != '\0') {
        uint8_t text_r = (button->text_color >> 16) & 0xFF;
        uint8_t text_g = (button->text_color >> 8) & 0xFF;
        uint8_t text_b = button->text_color & 0xFF;
        uint8_t text_a = (button->text_color >> 24) & 0xFF;
        
        // 创建Pango布局
        if (!button->layout) {
            button->layout = pango_cairo_create_layout(wayland_ctx->cairo_context);
            pango_layout_set_text(button->layout, button->text, -1);
            pango_layout_set_font_description(button->layout, pango_font_description_from_string("Sans 12"));
        } else {
            pango_layout_set_text(button->layout, button->text, -1);
        }
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
        
        // 计算文本位置（居中）
        int text_width, text_height;
        pango_layout_get_size(button->layout, &text_width, &text_height);
        int x_pos = self->x + (self->width - text_width/1000) / 2;
        int y_pos = self->y + (self->height - text_height/1000) / 2;
        
        cairo_move_to(wayland_ctx->cairo_context, x_pos, y_pos);
        pango_cairo_show_layout(wayland_ctx->cairo_context, button->layout);
    }
    
    cairo_restore(wayland_ctx->cairo_context);
    
    // 绘制子节点
    for (int i = 0; i < self->children.count; i++) {
        self->children.items[i]->node_draw(self->children.items[i]);
    }
}

void button_set_text(struct Button* button, const char* text) {
    if (!button) return;
    
    if (button->text) {
        free(button->text);
    }
    
    button->text = text ? strdup(text) : strdup("");
    
    // 重置Pango布局
    if (button->layout) {
        g_object_unref(button->layout);
        button->layout = NULL;
    }
}

void button_set_text_color(struct Button* button, uint32_t color) {
    if (!button) return;
    button->text_color = color;
}

void button_set_border_color(struct Button* button, uint32_t color) {
    if (!button) return;
    button->border_color = color;
}

void button_set_border_width(struct Button* button, int width) {
    if (!button) return;
    if (width < 0) width = 0;
    button->border_width = width;
}

void button_set_on_click(struct Button* button, void (*on_click)(struct Button* button), void* user_data) {
    if (!button) return;
    button->on_click = on_click;
    button->user_data = user_data;
}

void button_handle_click(struct Button* button) {
    if (button && button->on_click) {
        button->on_click(button);
    }
}
