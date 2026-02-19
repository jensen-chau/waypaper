#include "checkbox.h"
#include "context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <glib.h>

struct CheckBox* checkbox_new(const char* id, int x, int y, const char* text) {
    struct CheckBox* checkbox = (struct CheckBox*)malloc(sizeof(struct CheckBox));
    if (!checkbox) {
        fprintf(stderr, "Failed to allocate checkbox\n");
        return NULL;
    }
    
    // 初始化node
    node_init(&checkbox->node, id, x, y, 150, 25, Node); // 默认大小，后续会调整
    
    // 初始化checkbox属性
    checkbox->text = text ? strdup(text) : strdup("");
    checkbox->text_color = 0xFFFFFFFF; // 白色
    checkbox->background_color = 0xFF333333; // 深灰色背景
    checkbox->border_color = 0xFF666666; // 灰色边框
    checkbox->border_width = 1;
    checkbox->checked = 0;
    checkbox->on_toggle = NULL;
    checkbox->user_data = NULL;
    checkbox->layout = NULL;
    
    checkbox->node.node_draw = draw_checkbox;
    
    return checkbox;
}

void draw_checkbox(struct Node* self) {
    if (!self || self->node_type != Node) { // CheckBox继承自Node
        return;
    }
    
    struct CheckBox* checkbox = (struct CheckBox*)self;
    struct Context* context = get_context(0, 0);
    if (!context) {
        return;
    }
    
    struct WaylandContext* wayland_ctx = context->wayland_context;
    
    if (!wayland_ctx || !wayland_ctx->cairo_context) {
        // 回退到简单的矩形绘制
        for (int i = 0; i < self->width; i++) {
            for (int j = 0; j < self->height; j++) {
                draw_point(self->x + i, self->y + j, checkbox->background_color);
            }
        }
        return;
    }
    
    // 绘制背景
    uint8_t bg_r = (checkbox->background_color >> 16) & 0xFF;
    uint8_t bg_g = (checkbox->background_color >> 8) & 0xFF;
    uint8_t bg_b = checkbox->background_color & 0xFF;
    uint8_t bg_a = (checkbox->background_color >> 24) & 0xFF;
    
    cairo_save(wayland_ctx->cairo_context);
    cairo_set_source_rgba(wayland_ctx->cairo_context, bg_r/255.0, bg_g/255.0, bg_b/255.0, bg_a/255.0);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_fill(wayland_ctx->cairo_context);
    
    // 绘制边框
    uint8_t border_r = (checkbox->border_color >> 16) & 0xFF;
    uint8_t border_g = (checkbox->border_color >> 8) & 0xFF;
    uint8_t border_b = checkbox->border_color & 0xFF;
    uint8_t border_a = (checkbox->border_color >> 24) & 0xFF;
    
    cairo_set_source_rgba(wayland_ctx->cairo_context, border_r/255.0, border_g/255.0, border_b/255.0, border_a/255.0);
    cairo_set_line_width(wayland_ctx->cairo_context, checkbox->border_width);
    cairo_rectangle(wayland_ctx->cairo_context, self->x, self->y, self->width, self->height);
    cairo_stroke(wayland_ctx->cairo_context);
    
    // 绘制复选框区域（左侧）
    int checkbox_size = 18;
    int checkbox_x = self->x + 4;
    int checkbox_y = self->y + (self->height - checkbox_size) / 2;
    
    // 绘制复选框背景
    cairo_set_source_rgba(wayland_ctx->cairo_context, bg_r/255.0, bg_g/255.0, bg_b/255.0, bg_a/255.0);
    cairo_rectangle(wayland_ctx->cairo_context, checkbox_x, checkbox_y, checkbox_size, checkbox_size);
    cairo_fill(wayland_ctx->cairo_context);
    
    // 绘制复选框边框
    cairo_set_source_rgba(wayland_ctx->cairo_context, border_r/255.0, border_g/255.0, border_b/255.0, border_a/255.0);
    cairo_set_line_width(wayland_ctx->cairo_context, 1);
    cairo_rectangle(wayland_ctx->cairo_context, checkbox_x, checkbox_y, checkbox_size, checkbox_size);
    cairo_stroke(wayland_ctx->cairo_context);
    
    // 如果选中，绘制勾号
    if (checkbox->checked) {
        uint8_t text_r = (checkbox->text_color >> 16) & 0xFF;
        uint8_t text_g = (checkbox->text_color >> 8) & 0xFF;
        uint8_t text_b = checkbox->text_color & 0xFF;
        uint8_t text_a = (checkbox->text_color >> 24) & 0xFF;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
        cairo_set_line_width(wayland_ctx->cairo_context, 2);
        
        // 绘制勾号
        cairo_move_to(wayland_ctx->cairo_context, checkbox_x + 4, checkbox_y + 9);
        cairo_line_to(wayland_ctx->cairo_context, checkbox_x + 8, checkbox_y + 13);
        cairo_line_to(wayland_ctx->cairo_context, checkbox_x + 14, checkbox_y + 6);
        cairo_stroke(wayland_ctx->cairo_context);
    }
    
    // 绘制文本
    if (checkbox->text && checkbox->text[0] != '\0') {
        uint8_t text_r = (checkbox->text_color >> 16) & 0xFF;
        uint8_t text_g = (checkbox->text_color >> 8) & 0xFF;
        uint8_t text_b = checkbox->text_color & 0xFF;
        uint8_t text_a = (checkbox->text_color >> 24) & 0xFF;
        
        // 创建Pango布局
        if (!checkbox->layout) {
            checkbox->layout = pango_cairo_create_layout(wayland_ctx->cairo_context);
            pango_layout_set_text(checkbox->layout, checkbox->text, -1);
            pango_layout_set_font_description(checkbox->layout, pango_font_description_from_string("Sans 12"));
        } else {
            pango_layout_set_text(checkbox->layout, checkbox->text, -1);
        }
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
        
        // 计算文本位置（右对齐到复选框）
        int x_pos = checkbox_x + checkbox_size + 6;
        int y_pos = self->y + (self->height - 12) / 2; // 垂直居中
        
        cairo_move_to(wayland_ctx->cairo_context, x_pos, y_pos);
        pango_cairo_show_layout(wayland_ctx->cairo_context, checkbox->layout);
    }
    
    cairo_restore(wayland_ctx->cairo_context);
}

void checkbox_set_checked(struct CheckBox* checkbox, int checked) {
    if (!checkbox) return;
    
    int old_checked = checkbox->checked;
    checkbox->checked = checked;
    
    // 如果状态改变了，调用回调函数
    if (old_checked != checked && checkbox->on_toggle) {
        checkbox->on_toggle(checkbox, checked);
    }
}

void checkbox_set_text(struct CheckBox* checkbox, const char* text) {
    if (!checkbox) return;
    
    if (checkbox->text) {
        free(checkbox->text);
    }
    
    checkbox->text = text ? strdup(text) : strdup("");
    
    // 重置Pango布局
    if (checkbox->layout) {
        g_object_unref(checkbox->layout);
        checkbox->layout = NULL;
    }
}

void checkbox_set_text_color(struct CheckBox* checkbox, uint32_t color) {
    if (!checkbox) return;
    checkbox->text_color = color;
}

void checkbox_set_background_color(struct CheckBox* checkbox, uint32_t color) {
    if (!checkbox) return;
    checkbox->background_color = color;
}

void checkbox_set_border_color(struct CheckBox* checkbox, uint32_t color) {
    if (!checkbox) return;
    checkbox->border_color = color;
}

void checkbox_set_border_width(struct CheckBox* checkbox, int width) {
    if (!checkbox) return;
    if (width < 0) width = 0;
    checkbox->border_width = width;
}

void checkbox_set_on_toggle(struct CheckBox* checkbox, void (*on_toggle)(struct CheckBox* checkbox, int checked), void* user_data) {
    if (!checkbox) return;
    checkbox->on_toggle = on_toggle;
    checkbox->user_data = user_data;
}