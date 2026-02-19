#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include "node.h"
#include "event.h"
#include "utils.h"

struct WaylandContext;

typedef struct {
    double x;
    double y;
} Point;


struct Context {
    Point mouse_pos;
    struct WaylandContext* wayland_context;
    HashMap node_map;
};

struct Context* get_context(int width, int height);

void app_exit();

void run(struct Node* root);

void draw_point(int x, int y, uint32_t color);

void handle_event(PointEvent point_event, void* data);

void add_node(struct Node* node);

Point get_mouse_point_pos();

struct Node* get_node(const char* id);

#endif
