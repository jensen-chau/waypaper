#include "box.h"
#include <stdio.h>

struct Box* box_new(Layout layout, int x, int y, int width, int height) {
    struct Box* box = (struct Box*)malloc(sizeof(struct Box));
    if (!box) {
        fprintf(stderr, "Failed to allocate box\n");
        return NULL;
    }
    
    // 初始化children数组
    box->children.count = 0;
    box->children.items = NULL;
    
    // 初始化node
    node_init(&box->node, x, y, width, height, Box);
    box->layout = layout;
    
    box->node.node_draw = draw_box;

    return box;
}

void draw_box(struct Node* self) {
    for (int i = 0; i < self->width; i++) {
        for (int j = 0; j < self->height; j++) {
            draw_point(self->x + i, self->y + j, self->bg_color);
        }
    }


    for (int i = 0; i < self->children.count; i++) {
        self->children.items[i]->node_draw(self->children.items[i]);
    }
}
