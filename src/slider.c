#include "slider.h"
#include "context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <glib.h>

struct Slider* slider_new(const char* id, int x, int y, int width, int height, float min_value, float max_value, float initial_value) {
    struct Slider* slider = (struct Slider*)malloc(sizeof(struct Slider));
    if (!slider) {
        fprintf(stderr, "Failed to allocate slider\n");
        return NULL;
    }
    
    // 初始化node
    node_init(&slider->node, id, x, y, width, height, Node); // Slider继承自Node
    
    // 初始化slider属性
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->value = initial_value;
    slider->track_color = 0xFF333333; // 深灰色轨道
    slider->fill_color = 0xFF6666FF; // 蓝色填充
    slider->thumb_color = 0xFFFFFFFF; // 白色滑块
    slider->thumb_size = 12;
    slider->orientation = 0; // 水平
    slider->on_change = NULL;
    slider->user_data = NULL;
    slider->layout = NULL;
    
    // 确保初始值在范围内
    if (slider->value < slider->min_value) slider->value = slider->min_value;
    if (slider->value > slider->max_value) slider->value = slider->max_value;
    
    slider->node.node_draw = draw_slider;
    
    return slider;
}

void draw_slider(struct Node* self) {
    if (!self || self->node_type != Node) { // Slider继承自Node
        return;
    }
    
    struct Slider* slider = (struct Slider*)self;
    struct Context* context = get_context(0, 0);
    if (!context) {
        return;
    }
    
    struct WaylandContext* wayland_ctx = context->wayland_context;
    
    if (!wayland_ctx || !wayland_ctx->cairo_context) {
        // 回退到简单的矩形绘制
        for (int i = 0; i < self->width; i++) {
            for (int j = 0; j < self->height; j++) {
                draw_point(self->x + i, self->y + j, slider->track_color);
            }
        }
        return;
    }
    
    // 绘制轨道
    uint8_t track_r = (slider->track_color >> 16) & 0xFF;
    uint8_t track_g = (slider->track_color >> 8) & 0xFF;
    uint8_t track_b = slider->track_color & 0xFF;
    uint8_t track_a = (slider->track_color >> 24) & 0xFF;
    
    cairo_save(wayland_ctx->cairo_context);
    cairo_set_source_rgba(wayland_ctx->cairo_context, track_r/255.0, track_g/255.0, track_b/255.0, track_a/255.0);
    
    if (slider->orientation == 0) { // 水平滑块
        int track_y = self->y + (self->height - 4) / 2;
        cairo_rectangle(wayland_ctx->cairo_context, self->x, track_y, self->width, 4);
        cairo_fill(wayland_ctx->cairo_context);
        
        // 计算填充部分
        float ratio = (slider->value - slider->min_value) / (slider->max_value - slider->min_value);
        int fill_width = (int)(self->width * ratio);
        
        uint8_t fill_r = (slider->fill_color >> 16) & 0xFF;
        uint8_t fill_g = (slider->fill_color >> 8) & 0xFF;
        uint8_t fill_b = slider->fill_color & 0xFF;
        uint8_t fill_a = (slider->fill_color >> 24) & 0xFF;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, fill_r/255.0, fill_g/255.0, fill_b/255.0, fill_a/255.0);
        cairo_rectangle(wayland_ctx->cairo_context, self->x, track_y, fill_width, 4);
        cairo_fill(wayland_ctx->cairo_context);
        
        // 计算滑块位置
        int thumb_x = self->x + fill_width - slider->thumb_size/2;
        int thumb_y = track_y - (slider->thumb_size - 4) / 2;
        
        // 绘制滑块
        uint8_t thumb_r = (slider->thumb_color >> 16) & 0xFF;
        uint8_t thumb_g = (slider->thumb_color >> 8) & 0xFF;
        uint8_t thumb_b = slider->thumb_color & 0xFF;
        uint8_t thumb_a = (slider->thumb_color >> 24) & 0xFF;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, thumb_r/255.0, thumb_g/255.0, thumb_b/255.0, thumb_a/255.0);
        cairo_arc(wayland_ctx->cairo_context, thumb_x + slider->thumb_size/2, thumb_y + slider->thumb_size/2, slider->thumb_size/2, 0, 2 * M_PI);
        cairo_fill(wayland_ctx->cairo_context);
    } else { // 垂直滑块
        int track_x = self->x + (self->width - 4) / 2;
        cairo_rectangle(wayland_ctx->cairo_context, track_x, self->y, 4, self->height);
        cairo_fill(wayland_ctx->cairo_context);
        
        // 计算填充部分
        float ratio = (slider->value - slider->min_value) / (slider->max_value - slider->min_value);
        int fill_height = (int)(self->height * ratio);
        
        uint8_t fill_r = (slider->fill_color >> 16) & 0xFF;
        uint8_t fill_g = (slider->fill_color >> 8) & 0xFF;
        uint8_t fill_b = slider->fill_color & 0xFF;
        uint8_t fill_a = (slider->fill_color >> 24) & 0xFF;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, fill_r/255.0, fill_g/255.0, fill_b/255.0, fill_a/255.0);
        cairo_rectangle(wayland_ctx->cairo_context, track_x, self->y + self->height - fill_height, 4, fill_height);
        cairo_fill(wayland_ctx->cairo_context);
        
        // 计算滑块位置
        int thumb_x = track_x - (slider->thumb_size - 4) / 2;
        int thumb_y = self->y + self->height - fill_height - slider->thumb_size/2;
        
        // 绘制滑块
        uint8_t thumb_r = (slider->thumb_color >> 16) & 0xFF;
        uint8_t thumb_g = (slider->thumb_color >> 8) & 0xFF;
        uint8_t thumb_b = slider->thumb_color & 0xFF;
        uint8_t thumb_a = (slider->thumb_color >> 24) & 0xFF;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, thumb_r/255.0, thumb_g/255.0, thumb_b/255.0, thumb_a/255.0);
        cairo_arc(wayland_ctx->cairo_context, thumb_x + slider->thumb_size/2, thumb_y + slider->thumb_size/2, slider->thumb_size/2, 0, 2 * M_PI);
        cairo_fill(wayland_ctx->cairo_context);
    }
    
    cairo_restore(wayland_ctx->cairo_context);
}

void slider_set_value(struct Slider* slider, float value) {
    if (!slider) return;
    
    // 确保值在范围内
    if (value < slider->min_value) value = slider->min_value;
    if (value > slider->max_value) value = slider->max_value;
    
    float old_value = slider->value;
    slider->value = value;
    
    // 如果值改变了，调用回调函数
    if (old_value != value && slider->on_change) {
        slider->on_change(slider, value);
    }
}

void slider_set_min_value(struct Slider* slider, float min_value) {
    if (!slider) return;
    slider->min_value = min_value;
    
    // 确保当前值在新范围内
    if (slider->value < slider->min_value) {
        slider->value = slider->min_value;
    }
}

void slider_set_max_value(struct Slider* slider, float max_value) {
    if (!slider) return;
    slider->max_value = max_value;
    
    // 确保当前值在新范围内
    if (slider->value > slider->max_value) {
        slider->value = slider->max_value;
    }
}

void slider_set_track_color(struct Slider* slider, uint32_t color) {
    if (!slider) return;
    slider->track_color = color;
}

void slider_set_fill_color(struct Slider* slider, uint32_t color) {
    if (!slider) return;
    slider->fill_color = color;
}

void slider_set_thumb_color(struct Slider* slider, uint32_t color) {
    if (!slider) return;
    slider->thumb_color = color;
}

void slider_set_thumb_size(struct Slider* slider, int size) {
    if (!slider) return;
    if (size < 0) size = 0;
    slider->thumb_size = size;
}

void slider_set_orientation(struct Slider* slider, int orientation) {
    if (!slider) return;
    slider->orientation = orientation;
}

void slider_set_on_change(struct Slider* slider, void (*on_change)(struct Slider* slider, float value), void* user_data) {
    if (!slider) return;
    slider->on_change = on_change;
    slider->user_data = user_data;
}
