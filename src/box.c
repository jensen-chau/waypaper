#include "box.h"
#include <stdio.h>

struct Box* box_new(const char* id, Layout layout, int x, int y, int width, int height) {
    struct Box* box = (struct Box*)malloc(sizeof(struct Box));
    if (!box) {
        fprintf(stderr, "Failed to allocate box\n");
        return NULL;
    }
    
    // 初始化children数组
    box->children.count = 0;
    box->children.items = NULL;
    
    // 初始化node
    node_init(&box->node, id, x, y, width, height, Box);
    box->layout = layout;
    
    box->node.node_draw = draw_box;

    return box;
}

void draw_box(struct Node* self) {
    // 检查节点是否在视口内
    if (self->x >= 200 || self->y >= 200 || 
        self->x + self->width <= 0 || self->y + self->height <= 0) {
        return;
    }
    
    // 绘制节点背景
    for (int i = 0; i < self->width; i++) {
        for (int j = 0; j < self->height; j++) {
            int px = self->x + i;
            int py = self->y + j;
            
            // 边界检查
            if (px >= 0 && px < 200 && py >= 0 && py < 200) {
                draw_point(px, py, self->bg_color);
            }
        }
    }

    // 绘制子节点
    for (int i = 0; i < self->children.count; i++) {
        self->children.items[i]->node_draw(self->children.items[i]);
    }
}
