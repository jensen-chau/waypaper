#define STB_IMAGE_IMPLEMENTATION
#include "context.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "stb_image.h"
#include "stb_image_resize2.h"
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
    return ctx;
}

void app_exit() {
    should_exit = 1;
}

void load_wallpaper(const char* path) {
    int width, height;
    int channels_in_file;
    unsigned char* img_data =
        stbi_load(path, &width, &height, &channels_in_file, 4);
    if (img_data == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return;
    }
    printf("Image loaded: %dx%d, channels:%d\n", width, height,
           channels_in_file);

    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    
    // 为每个输出创建适当的壁纸
    for (int i = 0; i < wayland_ctx->num_outputs; i++) {
        OutputInfo* output_info = &wayland_ctx->outputs[i];
        int output_width = output_info->width;
        int output_height = output_info->height;
        
        printf("Processing output %d: %dx%d\n", i, output_width, output_height);
        
        // 缩放到当前输出的尺寸
        unsigned char* resized_data = malloc(output_width * output_height * 4);
        if (!resized_data) {
            fprintf(stderr, "Failed to allocate memory for resized image for output %d\n", i);
            continue;
        }

        // 使用stb_image_resize进行缩放
        stbir_resize_uint8_linear(img_data, width, height, 0,
                                  resized_data, output_width, output_height, 0,
                                  STBIR_4CHANNEL);
        printf("Image resized for output %d to: %dx%d\n", i, output_width, output_height);

        // 由于Wayland的ARGB8888在小端序系统上实际是BGRA字节序，需要转换颜色通道
        // 将RGBA转换为BGRA，以正确显示颜色
        for (int j = 0; j < output_width * output_height; j++) {
            unsigned char r = resized_data[j * 4 + 0];
            unsigned char b = resized_data[j * 4 + 2];
            resized_data[j * 4 + 0] = b; // R <- B
            resized_data[j * 4 + 2] = r; // B <- R
            // G 和 A 保持不变
        }

        // 为当前输出创建池
        create_pool_for_output(wayland_ctx, i, output_width, output_height, 4);

        // 将转换后的图片数据复制到SHM缓冲区
        if (output_info->shm_data) {
            memcpy(output_info->shm_data, resized_data, 
                   output_width * output_height * 4);
            printf("Resized image data copied to SHM buffer for output %d\n", i);
        }

        // 释放当前输出的临时数据
        free(resized_data);
    }

    // 释放原始图像数据
    stbi_image_free(img_data);
}

void run() {
    if (!ctx) {
        fprintf(stderr, "Error: context is NULL\n");
        return;
    }

    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Error: wayland context is NULL\n");
        return;
    }

    uint32_t cnt = 0;

    load_wallpaper("/home/zjx/Pictures/wallpaper/01.jpg");

    while (!should_exit) {
        if (++cnt % 60 == 0) {
            printf("frame: %d, outputs: %d\n", cnt, wayland_ctx->num_outputs);
        }

        // 为每个输出提交表面更改
        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];
            
            // 检查缓冲区是否有效
            if (!output_info->buffer) {
                fprintf(stderr, "Error: buffer is NULL for output %d\n", i);
                continue;
            }

            wl_surface_attach(output_info->surface, output_info->buffer, 0, 0);
            wl_surface_damage(output_info->surface, 0, 0, output_info->width,
                              output_info->height);
            wl_surface_commit(output_info->surface);
        }

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

void handle_event(PointEvent point_event, void* data) {
    printf("point_event:%d\n", point_event);
}

Point get_mouse_point_pos() {
    return ctx->mouse_pos;
}
