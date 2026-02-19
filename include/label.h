#ifndef LABEL_H
#define LABEL_H

#include "node.h"
#include "context.h"
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

// 前向声明
struct WaylandContext;

struct Label {
    struct Node node;
    char* text;
    uint32_t text_color;
    PangoLayout* layout;
};

struct Label* label_new(const char* id, int x, int y, const char* text);

void draw_label(struct Node* self);

void label_set_text(struct Label* label, const char* text);

void label_set_text_color(struct Label* label, uint32_t color);

#endif