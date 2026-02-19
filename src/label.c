#include "label.h"
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

struct Label* label_new(const char* id, int x, int y, const char* text) {
    struct Label* label = (struct Label*)malloc(sizeof(struct Label));
    if (!label) {
        fprintf(stderr, "Failed to allocate label\n");
        return NULL;
    }
    
    // 初始化node
    node_init(&label->node, id, x, y, 100, 20, Node); // 默认大小，后续会调整
    
    // 初始化标签属性
    label->text = text ? strdup(text) : strdup("");
    label->text_color = 0xFFFFFFFF; // 白色
    label->layout = NULL;
    
    label->node.node_draw = draw_label;
    
    return label;
}

void draw_label(struct Node* self) {
    if (!self || self->node_type != Node) { // Label继承自Node
        return;
    }
    
    struct Label* label = (struct Label*)self;
    struct Context* context = get_context(0, 0);
    if (!context) {
        return;
    }
    
    struct WaylandContext* wayland_ctx = context->wayland_context;
    
    if (!wayland_ctx || !wayland_ctx->cairo_context) {
        // 回退到简单的文本绘制
        if (label->text && label->text[0] != '\0') {
            for (int i = 0; label->text[i] != '\0' && i < self->width; i++) {
                // 这里可以实现简单的字符绘制
                draw_point(self->x + i * 6, self->y, label->text_color);
            }
        }
        return;
    }
    
    // 绘制文本
    if (label->text && label->text[0] != '\0') {
        uint8_t text_r = (label->text_color >> 16) & 0xFF;
        uint8_t text_g = (label->text_color >> 8) & 0xFF;
        uint8_t text_b = label->text_color & 0xFF;
        uint8_t text_a = (label->text_color >> 24) & 0xFF;
        
        // 创建Pango布局
        if (!label->layout) {
            label->layout = pango_cairo_create_layout(wayland_ctx->cairo_context);
            pango_layout_set_text(label->layout, label->text, -1);
            pango_layout_set_font_description(label->layout, pango_font_description_from_string("Sans 12"));
        } else {
            pango_layout_set_text(label->layout, label->text, -1);
        }
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, text_r/255.0, text_g/255.0, text_b/255.0, text_a/255.0);
        
        // 计算文本位置（左对齐）
        int x_pos = self->x;
        int y_pos = self->y;
        
        cairo_move_to(wayland_ctx->cairo_context, x_pos, y_pos);
        pango_cairo_show_layout(wayland_ctx->cairo_context, label->layout);
        
        // 更新节点高度为文本高度
        int text_height;
        pango_layout_get_size(label->layout, NULL, &text_height);
        self->height = text_height / 1000 + 4; // 加上一些边距
    }
}

void label_set_text(struct Label* label, const char* text) {
    if (!label) return;
    
    if (label->text) {
        free(label->text);
    }
    
    label->text = text ? strdup(text) : strdup("");
    
    // 重置Pango布局
    if (label->layout) {
        g_object_unref(label->layout);
        label->layout = NULL;
    }
}

void label_set_text_color(struct Label* label, uint32_t color) {
    if (!label) return;
    label->text_color = color;
}