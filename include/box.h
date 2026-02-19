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
    struct VectorNode children;
};

extern struct Box* box_new(const char* id, Layout layout, int x, int y, int width, int height);

extern void draw_box(struct Node* self);



#endif
