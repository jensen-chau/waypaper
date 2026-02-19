#include "text_input.h"
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

struct TextInput* text_input_new(const char* id, int x, int y, int width, int height, const char* placeholder) {
    struct TextInput* text_input = (struct TextInput*)malloc(sizeof(struct TextInput));
    if (!text_input) {
        fprintf(stderr, "Failed to allocate text_input\n");
        return NULL;
    }
    
    // 初始化node
    node_init(&text_input->node, id, x, y, width, height, Node); // TextInput继承自Node
    
    // 初始化文本输入属性
    text_input->text = placeholder ? strdup(placeholder) : strdup("");
    text_input->text_color = 0xFFFFFFFF; // 白色
    text_input->background_color = 0xFF333333; // 深灰色背景
    text_input->border_color = 0xFF666666; // 灰色边框
    text_input->border_width = 1;
    text_input->cursor_pos = 0;
    text_input->cursor_visible = 1;
    text_input->layout = NULL;
    
    text_input->node.node_draw = draw_text_input;
    
    return text_input;
}

void draw_text_input(struct Node* self) {
    if (!self || self->node_type != Node) { // TextInput继承自Node
        return;
    }
    
    struct TextInput* text_input = (struct TextInput*)self;
    struct Context* context = get_context(0, 0);
    if (!context) {
        return;
    }
    
    struct WaylandContext* wayland_ctx = context->wayland_context;
    
    if (!wayland_ctx || !wayland_ctx->cairo_context) {
        // 回退到简单的矩形绘制
        for (int i = 0; i < self->width; i++) {
            for (int j = 0; j < self->height; j++) {
                draw_point(self->x + i, self->y + j, text_input->background_color);
            }
        }
        return;
    }
    
    // 绘制背景
    uint8_t bg_r = (text_input->background_color >> 16) & 0xFF;
    uint8_t bg_g = (text_input->background_color >> 8) & 0xFF;
    uint8_t bg_b = text_input->background_color & 0xFF;
    uint8_t bg_a = (text_input->background_color >> 24) & 0xFF;
    
    cairo_save(wayland_ctx->cairo_context);
    cairo_set_source_rgba(wayland_ctx->cairo_context, bg_r/255.0, bg_g/255.0, bg_b/255.0, bg_a/255.0);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_fill(wayland_ctx->cairo_context);
    
    // 绘制边框
    uint8_t border_r = (text_input->border_color >> 16) & 0xFF;
    uint8_t border_g = (text_input->border_color >> 8) & 0xFF;
    uint8_t border_b = text_input->border_color & 0xFF;
    uint8_t border_a = (text_input->border_color >> 24) & 0xFF;
    
    cairo_set_source_rgba(wayland_ctx->cairo_context, border_r/255.0, border_g/255.0, border_b/255.0, border_a/255.0);
    cairo_set_line_width(wayland_ctx->cairo_context, text_input->border_width);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_stroke(wayland_ctx->cairo_context);
    
    // 绘制文本
    if (text_input->text && text_input->text[0] != '\0') {
        uint8_t text_r = (text_input->text_color >> 16) & 0xFF;
        uint8_t text_g = (text_input->text_color >> 8) & 0xFF;
        uint8_t text_b = text_input->text_color & 0xFF;
        uint8_t text_a = (text_input->text_color >> 24) & 0xFF;
        
        // 创建Pango布局
        if (!text_input->layout) {
            text_input->layout = pango_cairo_create_layout(wayland_ctx->cairo_context);
            pango_layout_set_text(text_input->layout, text_input->text, -1);
            pango_layout_set_font_description(text_input->layout, pango_font_description_from_string("Sans 12"));
        } else {
            pango_layout_set_text(text_input->layout, text_input->text, -1);
        }
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
        
        // 计算文本位置（左对齐，稍微向右偏移）
        int x_pos = self->x + 4;
        int y_pos = self->y + (self->height - 12) / 2; // 垂直居中
        
        cairo_move_to(wayland_ctx->cairo_context, x_pos, y_pos);
        pango_cairo_show_layout(wayland_ctx->cairo_context, text_input->layout);
        
        // 绘制光标
        if (text_input->cursor_visible) {
            int cursor_x = x_pos + text_input->cursor_pos * 6; // 简单的字符宽度估算
            if (cursor_x < self->x + self->width - 4) {
                cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
                cairo_rectangle(wayland_ctx->cairo_context, cursor_x, self->y + 2, 1, self->height - 4);
                cairo_fill(wayland_ctx->cairo_context);
            }
        }
    }
    
    cairo_restore(wayland_ctx->cairo_context);
}

void text_input_set_text(struct TextInput* text_input, const char* text) {
    if (!text_input) return;
    
    if (text_input->text) {
        free(text_input->text);
    }
    
    text_input->text = text ? strdup(text) : strdup("");
    
    // 重置Pango布局
    if (text_input->layout) {
        g_object_unref(text_input->layout);
        text_input->layout = NULL;
    }
}

void text_input_set_placeholder(struct TextInput* text_input, const char* placeholder) {
    if (!text_input) return;
    
    if (text_input->text && text_input->text[0] == '\0') {
        if (text_input->text) {
            free(text_input->text);
        }
        text_input->text = placeholder ? strdup(placeholder) : strdup("");
    }
}

void text_input_set_text_color(struct TextInput* text_input, uint32_t color) {
    if (!text_input) return;
    text_input->text_color = color;
}

void text_input_set_background_color(struct TextInput* text_input, uint32_t color) {
    if (!text_input) return;
    text_input->background_color = color;
}

void text_input_set_border_color(struct TextInput* text_input, uint32_t color) {
    if (!text_input) return;
    text_input->border_color = color;
}

void text_input_set_border_width(struct TextInput* text_input, int width) {
    if (!text_input) return;
    if (width < 0) width = 0;
    text_input->border_width = width;
}
