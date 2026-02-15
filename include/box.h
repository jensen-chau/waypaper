#ifndef BOX_H
#define BOX_H

#include "node.h"

typedef enum Layout {
    Vertical,
    Horizontal
} Layout;

struct Box {
    struct Node;
    Layout layout;
};

void box_init(struct Box* self, Layout layout, int x) {
    
    self->layout = layout;
}

#endif
