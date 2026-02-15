#ifndef BOX_H
#define BOX_H

#include "node.h"
#include <stdlib.h>

typedef enum Layout {
    Vertical,
    Horizontal
} Layout;

struct Box {
    struct Node node;
    Layout layout;
};

void box_init(struct Box* self, Layout layout, int x, int y, int width, int height) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node_init(node, x, y, width, height, Box);
    self->layout = layout;
}

#endif
