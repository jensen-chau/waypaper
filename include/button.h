#ifndef BUTTON_H
#define BUTTON_H

#include "node.h"
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <glib.h>

// 前向声明
struct WaylandContext;

struct Button;


struct Button {
    struct Node node;
    char* text;
    uint32_t text_color;
    uint32_t border_color;
    int border_width;
    PangoLayout* layout;
    void (*on_click)(struct Button* button);
    void* user_data;
};

struct Button* button_new(const char* id, int x, int y, int width, int height, const char* text);

void draw_button(struct Node* self);

int is_hoverd(struct Node* self);

void button_set_text(struct Button* button, const char* text);

void button_set_text_color(struct Button* button, uint32_t color);

void button_set_border_color(struct Button* button, uint32_t color);

void button_set_border_width(struct Button* button, int width);

void button_set_on_click(struct Button* button, void (*on_click)(struct Button* button), void* user_data);

#endif
