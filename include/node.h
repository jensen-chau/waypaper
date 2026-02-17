#ifndef NODE_H
#define NODE_H

#include <wayland-client-protocol.h>
#include "event.h"
#include <stdlib.h>

struct Context;

void draw_point(int x, int y, unsigned int color);

#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

#define da_append(vi, x)                                                       \
  do {                                                                         \
    if (vi.count >= vi.capacity) {                                             \
      if (vi.capacity == 0)                                                    \
        vi.capacity = 5;                                                     \
      vi.capacity *= 2;                                                        \
      vi.items = realloc(vi.items, vi.capacity * sizeof(*vi.items));           \
    }                                                                          \
    vi.items[vi.count++] = x;                                                  \
  } while (0)

struct VectorNode {
    struct Node** items;
    int capacity;
    int count;
};


typedef void (*NodeDraw)(struct Node* self);

typedef void (*HandleEvent)(struct Node* self, EVENT event);

typedef int (*AddChild)(struct Node* self, struct Node child);

typedef enum NodeType {
    Node,
    Box,
    Button,
}NodeType;

struct Node {
    int x;
    int y;
    int width;
    int height;
    uint32_t bg_color;
    struct VectorNode children;
    NodeType node_type;
    NodeDraw node_draw;
    HandleEvent handle_event;
    AddChild add_child;
};

extern void node_init(struct Node* self, int x, int y, int width, int height, NodeType node_type);



extern void node_draw(struct Node* self);



extern void add_child(struct Node* self, struct Node* child);



extern void set_bg_color(struct Node* self, uint32_t color);

#endif
