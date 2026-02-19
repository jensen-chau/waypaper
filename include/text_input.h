#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include "node.h"
#include "context.h"
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

// 前向声明
struct WaylandContext;

struct TextInput {
    struct Node node;
    char* text;
    uint32_t text_color;
    uint32_t background_color;
    uint32_t border_color;
    int border_width;
    int cursor_pos;
    int cursor_visible;
    PangoLayout* layout;
};

struct TextInput* text_input_new(const char* id, int x, int y, int width, int height, const char* placeholder);

void draw_text_input(struct Node* self);

void text_input_set_text(struct TextInput* text_input, const char* text);

void text_input_set_placeholder(struct TextInput* text_input, const char* placeholder);

void text_input_set_text_color(struct TextInput* text_input, uint32_t color);

void text_input_set_background_color(struct TextInput* text_input, uint32_t color);

void text_input_set_border_color(struct TextInput* text_input, uint32_t color);

void text_input_set_border_width(struct TextInput* text_input, int width);

#endif