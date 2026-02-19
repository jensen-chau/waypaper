#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "node.h"
#include "context.h"
#include "wayland_context.h"
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

struct CheckBox {
    struct Node node;
    char* text;
    uint32_t text_color;
    uint32_t background_color;
    uint32_t border_color;
    int border_width;
    int checked;
    void (*on_toggle)(struct CheckBox* checkbox, int checked);
    void* user_data;
    PangoLayout* layout;
};

struct CheckBox* checkbox_new(const char* id, int x, int y, const char* text);

void draw_checkbox(struct Node* self);

void checkbox_set_checked(struct CheckBox* checkbox, int checked);

void checkbox_set_text(struct CheckBox* checkbox, const char* text);

void checkbox_set_text_color(struct CheckBox* checkbox, uint32_t color);

void checkbox_set_background_color(struct CheckBox* checkbox, uint32_t color);

void checkbox_set_border_color(struct CheckBox* checkbox, uint32_t color);

void checkbox_set_border_width(struct CheckBox* checkbox, int width);

void checkbox_set_on_toggle(struct CheckBox* checkbox, void (*on_toggle)(struct CheckBox* checkbox, int checked), void* user_data);

#endif