#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include "node.h"

struct WaylandContext;

typedef struct {
    float x;
    float y;
} Point;


struct Context {
    Point mouse_pos;
    struct WaylandContext* wayland_context;
};

struct Context* get_context(int width, int height);

void app_exit();

void run(struct Node* root);

void draw_point(int x, int y, uint32_t color);


#endif
