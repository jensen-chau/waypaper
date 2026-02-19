#ifndef SLIDER_H
#define SLIDER_H

#include "node.h"
#include "wayland_context.h"
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

struct Slider {
    struct Node node;
    float min_value;
    float max_value;
    float value;
    uint32_t track_color;
    uint32_t fill_color;
    uint32_t thumb_color;
    int thumb_size;
    int orientation; // 0 = horizontal, 1 = vertical
    void (*on_change)(struct Slider* slider, float value);
    void* user_data;
    PangoLayout* layout;
};

struct Slider* slider_new(const char* id, int x, int y, int width, int height, float min_value, float max_value, float initial_value);

void draw_slider(struct Node* self);

void slider_set_value(struct Slider* slider, float value);

void slider_set_min_value(struct Slider* slider, float min_value);

void slider_set_max_value(struct Slider* slider, float max_value);

void slider_set_track_color(struct Slider* slider, uint32_t color);

void slider_set_fill_color(struct Slider* slider, uint32_t color);

void slider_set_thumb_color(struct Slider* slider, uint32_t color);

void slider_set_thumb_size(struct Slider* slider, int size);

void slider_set_orientation(struct Slider* slider, int orientation);

void slider_set_on_change(struct Slider* slider, void (*on_change)(struct Slider* slider, float value), void* user_data);

#endif