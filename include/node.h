#ifndef NODE_H
#define NODE_H

#include <wayland-client-protocol.h>
#include "context.h"
#include "event.h"
#include <stdlib.h>

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

void node_init(struct Node* self, int x, int y, int width, int height, NodeType node_type) {
    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;
    self->node_type = node_type;
}

void node_draw(struct Node* self) {
    for(int i=0; i<self->width; i++) {
        for (int j=0; j<self->height; i++) {
            draw_point(self->x + i, self->y + j, 0xAAFF3200);
        }
    }
}

void add_child(struct Node* self, struct Node* child) {
    da_append(self->children, child);
}


void set_bg_color(struct Node* self, uint32_t color) {
    self->bg_color = color;
}

#endif
