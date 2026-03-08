#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
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
};

struct Context* get_context(int width, int height);

void run();

int load_wallpaper(const char* path);

void draw_point(int x, int y, uint32_t color);

void handle_event(PointEvent point_event, void* data);

Point get_mouse_point_pos();


#endif
