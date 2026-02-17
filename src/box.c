#include "box.h"

struct Box* box_new(Layout layout, int x, int y, int width, int height) {
    struct Box* box = (struct Box*)malloc(sizeof(struct Box));
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node_init(node, x, y, width, height, Box);
    box->layout = layout;
    return box;
}

void draw_box(struct Node* self) {
    struct Box* box = container_of(self, struct Box, node);
    for (int i = 0; i < self->width; i++) {
        for (int j = 0; j < self->height; j++) {
            draw_point(self->x + i, self->y + j, 0x00FF0000);
        }
    }

    for (int i = 0; i < box->children.count; i++) {
        box->children.items[i]->node_draw(box->children.items[i]);
    }
}