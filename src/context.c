#include "context.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include "utils.h"
#include "wayland_context.h"

struct Node;

static int should_exit = 0;
static struct Context* ctx = NULL;

struct Context* get_context(int width, int height) {
    if (ctx != NULL) {
        return ctx;
    }
    ctx = malloc(sizeof(struct Context));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate context\n");
        return NULL;
    }
    ctx->wayland_context = wayland_context_init(width, height);
    if (!ctx->wayland_context) {
        fprintf(stderr, "Failed to initialize wayland context\n");
        free(ctx);
        ctx = NULL;
        return NULL;
    }
    ctx->node_map = (HashMap){0};
    ctx->root_node = NULL;
    return ctx;
}

void app_exit() {
    should_exit = 1;
}

void run(struct Node* root) {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return;
    }
    
    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Error: wayland context is NULL\n");
        return;
    }
    
    if (!root) {
        fprintf(stderr, "Error: root node is NULL\n");
        return;
    }
    
    // 设置根节点
    ctx->root_node = root;

    while (!should_exit) {
        // 检查缓冲区是否有效
        if (!wayland_ctx->buffer) {
            fprintf(stderr, "Error: buffer is NULL\n");
            break;
        }

        // 清除Cairo surface
        if (wayland_ctx->cairo_context) {
            cairo_save(wayland_ctx->cairo_context);
            cairo_set_operator(wayland_ctx->cairo_context, CAIRO_OPERATOR_CLEAR);
            cairo_paint(wayland_ctx->cairo_context);
            cairo_restore(wayland_ctx->cairo_context);
        }

        root->node_draw(root);

        // 将Cairo内容绘制到Wayland buffer
        if (wayland_ctx->cairo_context) {
            cairo_surface_flush(wayland_ctx->cairo_surface);
        }

        wl_surface_attach(wayland_ctx->surface, wayland_ctx->buffer, 0, 0);
        wl_surface_damage(wayland_ctx->surface, 0, 0, wayland_ctx->width, wayland_ctx->height);
        wl_surface_commit(wayland_ctx->surface);

        // 处理待处理的 Wayland 事件
        wl_display_flush(wayland_ctx->display);
        while (wl_display_prepare_read(wayland_ctx->display) != 0) {
            wl_display_dispatch_pending(wayland_ctx->display);
        }
        wl_display_read_events(wayland_ctx->display);
        wl_display_dispatch_pending(wayland_ctx->display);

        usleep(16667);
    }

    wayland_context_cleanup(wayland_ctx);
    free(ctx);
    ctx = NULL;
}

void add_node(struct Node* node) {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return;
    }
    
    uint32_t key = str_to_num(node->id);

    hash_map_put(&ctx->node_map, key, node);

}

struct Node* get_node(const char* id) {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return NULL;
    }
    
    uint32_t key = str_to_num(id);

    return  (struct Node*)hash_map_get(&ctx->node_map, key);
}

void draw_point(int x, int y, uint32_t color) {
    if (!ctx || !ctx->wayland_context) {
        fprintf(stderr, "Error: Invalid context in draw_point\n");
        return;
    }
    
    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (x < 0 || x >= wayland_ctx->width || y < 0 || y >= wayland_ctx->height) {
        return; // 边界检查
    }
    
    // 使用Cairo渲染
    if (wayland_ctx->cairo_context) {
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        uint8_t a = (color >> 24) & 0xFF;
        
        // 如果alpha通道为0，设置为255（不透明）
        if (a == 0) a = 255;
        
        cairo_set_source_rgba(wayland_ctx->cairo_context, r/255.0, g/255.0, b/255.0, a/255.0);
        cairo_rectangle(wayland_ctx->cairo_context, x, y, 1, 1);
        cairo_fill(wayland_ctx->cairo_context);
    } else if (wayland_ctx->shm_data) {
        // 回退到原始的内存操作方式
        ((uint32_t*)wayland_ctx->shm_data)[x + y * wayland_ctx->width] = color;
    }
}


void handle_event(PointEvent point_event, void *data) {
    printf("point_event:%d\n", point_event);
    if (point_event == MOTION) {
        ctx->mouse_pos.x = ((Point*)data)->x;
        ctx->mouse_pos.y = ((Point*)data)->y;
    } else if (point_event == MOUSE_CLICK) {
        // 处理鼠标点击事件
        Point mouse_pos = ctx->mouse_pos;
        
        // 遍历所有节点，查找点击位置的节点
        struct Node* clicked_node = NULL;
        struct VectorNode* stack = malloc(sizeof(struct VectorNode));
        stack->items = malloc(sizeof(struct Node*) * 100); // 假设最多100个节点
        stack->count = 0;
        stack->capacity = 100;
        
        // 将根节点压入栈
        if (ctx->root_node) {
            stack->items[stack->count++] = ctx->root_node;
        }
        
        // 使用深度优先搜索查找点击位置的节点
        while (stack->count > 0) {
            struct Node* node = stack->items[--stack->count];
            
            // 检查点击位置是否在节点内
            if (mouse_pos.x >= node->x && mouse_pos.x < node->x + node->width &&
                mouse_pos.y >= node->y && mouse_pos.y < node->y + node->height) {
                clicked_node = node;
                
                // 将子节点压入栈，优先处理子节点（因为它们在上面）
                for (int i = node->children.count - 1; i >= 0; i--) {
                    if (stack->count < stack->capacity) {
                        stack->items[stack->count++] = node->children.items[i];
                    }
                }
            }
        }
        
        free(stack->items);
        free(stack);
        
        // 如果找到了点击的节点，触发点击事件
        if (clicked_node && clicked_node->node_type == Button) {
            struct Button* button = (struct Button*)clicked_node;
            button_handle_click(button);
        }
    }
}

Point get_mouse_point_pos() {
    return ctx->mouse_pos;
}
