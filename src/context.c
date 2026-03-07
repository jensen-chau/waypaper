#define STB_IMAGE_IMPLEMENTATION
#include "context.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "stb_image.h"
#include "scale.h"
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
    
    // 如果所有输出分辨率相同，可以优化处理
    int all_outputs_same = 1;
    if (wayland_ctx->num_outputs > 1) {
        int first_width = wayland_ctx->outputs[0].width;
        int first_height = wayland_ctx->outputs[0].height;
        for (int i = 1; i < wayland_ctx->num_outputs; i++) {
            if (wayland_ctx->outputs[i].width != first_width || 
                wayland_ctx->outputs[i].height != first_height) {
                all_outputs_same = 0;
                break;
            }
        }
    }
    
    if (all_outputs_same && wayland_ctx->num_outputs > 1) {
        // 如果所有输出分辨率相同，只需缩放一次
        OutputInfo* first_output = &wayland_ctx->outputs[0];
        int output_width = first_output->width;
        int output_height = first_output->height;
        
        printf("All outputs have same resolution: %dx%d, optimizing...\n", output_width, output_height);
        
        // 使用新的缩放模块进行缩放（填充模式：按比例缩放再裁切）
        int output_size;
        unsigned char* resized_data = scale_image(img_data, width, height, 
                                                 output_width, output_height, 
                                                 SCALE_MODE_FILL, &output_size);
        if (!resized_data) {
            fprintf(stderr, "Failed to scale image\n");
            stbi_image_free(img_data);
            return;
        }

        // 由于Wayland的ARGB8888在小端序系统上实际是BGRA字节序，需要转换颜色通道
        // 将RGBA转换为BGRA，以正确显示颜色
        for (int j = 0; j < output_width * output_height; j++) {
            unsigned char r = resized_data[j * 4 + 0];
            unsigned char b = resized_data[j * 4 + 2];
            resized_data[j * 4 + 0] = b; // R <- B
            resized_data[j * 4 + 2] = r; // B <- R
            // G 和 A 保持不变
        }

        // 为每个输出创建池并复制数据
        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];
            
            // 为当前输出创建池
            create_pool_for_output(wayland_ctx, i, output_width, output_height, 4);

            // 将转换后的图片数据复制到SHM缓冲区
            if (output_info->shm_data) {
                memcpy(output_info->shm_data, resized_data, 
                       output_width * output_height * 4);
                printf("Image data copied to SHM buffer for output %d\n", i);
            }
        }

        // 释放临时数据
        free(resized_data);
    } else {
        // 如果输出分辨率不同，为每个输出单独处理
        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];
            int output_width = output_info->width;
            int output_height = output_info->height;
            
            printf("Processing output %d: %dx%d\n", i, output_width, output_height);
            
            // 使用新的缩放模块进行缩放（填充模式：按比例缩放再裁切）
            int output_size;
            unsigned char* resized_data = scale_image(img_data, width, height, 
                                                     output_width, output_height, 
                                                     SCALE_MODE_FILL, &output_size);
            if (!resized_data) {
                fprintf(stderr, "Failed to scale image for output %d\n", i);
                continue;
            }

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

    load_wallpaper("/home/zjx/Pictures/wallpaper/01.jpg");

    int is_update = 1;

    while (!should_exit) {

        if (is_update == 0) {
            usleep(50000);
            continue;
        }

        // 为每个输出提交表面更改 - 只在初始化时或需要更新时执行
        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];
            
            // 检查缓冲区是否有效
            if (!output_info->buffer) {
                continue; // 跳过错误输出，而不是打印错误
            }

            wl_surface_attach(output_info->surface, output_info->buffer, 0, 0);
            wl_surface_damage_buffer(output_info->surface, 0, 0, output_info->width,
                              output_info->height);
            wl_surface_commit(output_info->surface);
        }

        // 只处理待处理的 Wayland 事件，避免阻塞
        wl_display_flush(wayland_ctx->display);
        
        // 使用 dispatch_queue 来更高效地处理事件
        int ret = wl_display_dispatch_pending(wayland_ctx->display);
        if (ret == -1) {
            break; // 错误，退出循环
        }
        
        is_update = 0;
        // 短暂休眠以减少CPU使用，但只在没有事件时
        usleep(50000); // 50ms instead of 16ms to reduce CPU usage
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
