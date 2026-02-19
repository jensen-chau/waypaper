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

    // 如果没有子节点，直接返回
    if (self->children.count == 0) {
        return;
    }

    // 获取box的布局类型
    struct Box* box = (struct Box*)self;
    
    // 计算布局
    if (box->layout == Vertical) {
        // 垂直布局：子节点从上到下排列
        int current_y = self->y;
        int spacing = 5; // 子节点之间的间距
        
        for (int i = 0; i < self->children.count; i++) {
            struct Node* child = self->children.items[i];
            
            // 调整子节点的位置和大小
            child->x = self->x;
            child->y = current_y;
            
            // 尝试计算子节点的宽度（使用父节点宽度）
            if (child->width == 0 || child->width > self->width) {
                child->width = self->width - 8; // 减去一些边距
            }
            
            // 调整子节点的高度（如果需要）
            if (child->height == 0) {
                child->height = 30; // 默认高度
            }
            
            // 绘制子节点
            child->node_draw(child);
            
            // 更新下一个子节点的Y位置
            current_y += child->height + spacing;
            
            // 如果超出边界，停止绘制
            if (current_y >= self->y + self->height) {
                break;
            }
        }
    } else if (box->layout == Horizontal) {
        // 水平布局：子节点从左到右排列
        int current_x = self->x;
        int spacing = 5; // 子节点之间的间距
        
        for (int i = 0; i < self->children.count; i++) {
            struct Node* child = self->children.items[i];
            
            // 调整子节点的位置和大小
            child->x = current_x;
            child->y = self->y;
            
            // 尝试计算子节点的高度（使用父节点高度）
            if (child->height == 0 || child->height > self->height) {
                child->height = self->height - 8; // 减去一些边距
            }
            
            // 调整子节点的宽度（如果需要）
            if (child->width == 0) {
                child->width = 80; // 默认宽度
            }
            
            // 绘制子节点
            child->node_draw(child);
            
            // 更新下一个子节点的X位置
            current_x += child->width + spacing;
            
            // 如果超出边界，停止绘制
            if (current_x >= self->x + self->width) {
                break;
            }
        }
    } else if (box->layout == Grid) {
        // 网格布局：子节点按网格排列
        int cols = 3; // 默认列数
        int rows = (self->children.count + cols - 1) / cols; // 计算行数
        
        int cell_width = (self->width - (cols - 1) * 5) / cols;
        int cell_height = (self->height - (rows - 1) * 5) / rows;
        
        for (int i = 0; i < self->children.count; i++) {
            struct Node* child = self->children.items[i];
            
            int row = i / cols;
            int col = i % cols;
            
            // 调整子节点的位置和大小
            child->x = self->x + col * (cell_width + 5);
            child->y = self->y + row * (cell_height + 5);
            child->width = cell_width;
            child->height = cell_height;
            
            // 绘制子节点
            child->node_draw(child);
        }
    } else if (box->layout == Stack) {
        // 堆叠布局：所有子节点重叠
        for (int i = 0; i < self->children.count; i++) {
            struct Node* child = self->children.items[i];
            
            // 调整子节点的位置和大小（与父节点相同）
            child->x = self->x;
            child->y = self->y;
            child->width = self->width;
            child->height = self->height;
            
            // 绘制子节点
            child->node_draw(child);
        }
    }
}
