#ifndef NODE_H
#define NODE_H

#include <setjmp.h>
#include <wayland-client-protocol.h>
#include "event.h"
struct Node;

typedef void (*NodeDraw)(struct Node* self);

typedef void (*HandleEvent)(struct Node* self, EVENT event);

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
    NodeType node_type;
    NodeDraw node_draw;
    HandleEvent handle_event;
};

void node_init(struct Node* self, int x, int y, int width, int height, NodeType node_type) {
    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;
    self->node_type = node_type;
} 

#endif
