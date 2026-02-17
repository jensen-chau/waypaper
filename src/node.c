#include "node.h"

void node_init(struct Node* self, int x, int y, int width, int height, NodeType node_type) {
    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;
    self->node_type = node_type;
}

void node_draw(struct Node* self) {
    for(int i=0; i<self->width; i++) {
        for (int j=0; j<self->height; j++) {
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