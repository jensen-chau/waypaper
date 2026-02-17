#include "node.h"
#include <stdio.h>

void node_init(struct Node* self, int x, int y, int width, int height, NodeType node_type) {
    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;
    self->node_type = node_type;
    self->node_draw = node_draw;
    self->add_child = add_child;
    self->children.count = 0;
    self->children.capacity = 0;
    self->children.items = NULL;
}

void node_draw(struct Node* self) {
    if (!self) {
        return;
    }
    
    // 使用硬编码的边界检查，因为无法直接访问全局ctx
    for(int i=0; i<self->width && i<200; i++) {
        for (int j=0; j<self->height && j<200; j++) {
            int px = self->x + i;
            int py = self->y + j;
            
            // 简单的边界检查
            if (px >= 0 && px < 200 && py >= 0 && py < 200) {
                draw_point(px, py, 0xAAFF3200);
            }
        }
    }
}

void add_child(struct Node* self, struct Node* child) {
    if (!self || !child) {
        return;
    }
    da_append(self->children, child);
    printf("children count: %d\n", self->children.count);
}

void set_bg_color(struct Node* self, uint32_t color) {
    if (!self) {
        return;
    }
    self->bg_color = color;
}
