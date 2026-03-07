#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "scale.h"
#include "stb_image_resize2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 按比例缩放并居中裁切图像（填充模式）
static unsigned char* scale_fill(const unsigned char* input_data,
                                 int input_width, int input_height,
                                 int output_width, int output_height,
                                 int* output_size) {
    // 计算按比例缩放后的尺寸
    float img_aspect = (float)input_width / input_height;
    float output_aspect = (float)output_width / output_height;
    
    int scale_width, scale_height;
    if (img_aspect > output_aspect) {
        // 图片更宽，以高度为基准缩放
        scale_height = output_height;
        scale_width = (int)(scale_height * img_aspect);
    } else {
        // 图片更高，以宽度为基准缩放
        scale_width = output_width;
        scale_height = (int)(scale_width / img_aspect);
    }
    
    printf("Scaling image to maintain aspect ratio: %dx%d -> %dx%d\n", 
           input_width, input_height, scale_width, scale_height);
    
    // 按比例缩放图像
    unsigned char* scaled_data = malloc(scale_width * scale_height * 4);
    if (!scaled_data) {
        fprintf(stderr, "Failed to allocate memory for scaled image in scale_fill\n");
        return NULL;
    }

    // 使用stb_image_resize进行按比例缩放
    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              scaled_data, scale_width, scale_height, 0,
                              STBIR_4CHANNEL);
    printf("Image scaled to maintain aspect ratio: %dx%d\n", scale_width, scale_height);

    // 裁切图像以匹配输出尺寸
    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        fprintf(stderr, "Failed to allocate memory for cropped image in scale_fill\n");
        free(scaled_data);
        return NULL;
    }
    
    // 计算裁切起始位置（居中裁切）
    int crop_start_x = (scale_width - output_width) / 2;
    int crop_start_y = (scale_height - output_height) / 2;
    
    printf("Crop start position: (%d, %d)\n", crop_start_x, crop_start_y);
    
    // 从缩放后的图像中裁切中央部分
    for (int y = 0; y < output_height; y++) {
        int src_row = (crop_start_y + y) * scale_width;
        int dst_row = y * output_width;
        for (int x = 0; x < output_width; x++) {
            int src_idx = (src_row + crop_start_x + x) * 4;
            int dst_idx = (dst_row + x) * 4;
            // 复制RGBA四个通道
            output_data[dst_idx + 0] = scaled_data[src_idx + 0]; // R/B
            output_data[dst_idx + 1] = scaled_data[src_idx + 1]; // G
            output_data[dst_idx + 2] = scaled_data[src_idx + 2]; // B/R
            output_data[dst_idx + 3] = scaled_data[src_idx + 3]; // A
        }
    }
    
    printf("Image cropped from %dx%d to %dx%d\n", scale_width, scale_height, output_width, output_height);

    // 释放临时数据
    free(scaled_data);
    
    *output_size = output_width * output_height * 4;
    return output_data;
}

// 适应模式：完整显示图像，可能有黑边
static unsigned char* scale_fit(const unsigned char* input_data,
                                int input_width, int input_height,
                                int output_width, int output_height,
                                int* output_size) {
    // 计算按比例缩放后的尺寸
    float img_aspect = (float)input_width / input_height;
    float output_aspect = (float)output_width / output_height;
    
    int scale_width, scale_height;
    if (img_aspect > output_aspect) {
        // 图片更宽，以宽度为基准缩放
        scale_width = output_width;
        scale_height = (int)(scale_width / img_aspect);
    } else {
        // 图片更高，以高度为基准缩放
        scale_height = output_height;
        scale_width = (int)(scale_height * img_aspect);
    }
    
    printf("Scaling image to fit: %dx%d -> %dx%d\n", 
           input_width, input_height, scale_width, scale_height);
    
    // 按比例缩放图像
    unsigned char* scaled_data = malloc(scale_width * scale_height * 4);
    if (!scaled_data) {
        fprintf(stderr, "Failed to allocate memory for scaled image in scale_fit\n");
        return NULL;
    }

    // 使用stb_image_resize进行按比例缩放
    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              scaled_data, scale_width, scale_height, 0,
                              STBIR_4CHANNEL);
    printf("Image scaled to fit: %dx%d\n", scale_width, scale_height);

    // 创建输出图像，初始化为黑色背景
    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        fprintf(stderr, "Failed to allocate memory for output image in scale_fit\n");
        free(scaled_data);
        return NULL;
    }
    
    // 初始化为黑色背景
    for (int i = 0; i < output_width * output_height * 4; i += 4) {
        output_data[i + 0] = 0; // R
        output_data[i + 1] = 0; // G
        output_data[i + 2] = 0; // B
        output_data[i + 3] = 255; // A
    }
    
    // 计算放置位置（居中）
    int start_x = (output_width - scale_width) / 2;
    int start_y = (output_height - scale_height) / 2;
    
    // 复制缩放后的图像到输出图像的中心
    for (int y = 0; y < scale_height; y++) {
        int src_row = y * scale_width;
        int dst_row = (start_y + y) * output_width;
        for (int x = 0; x < scale_width; x++) {
            int src_idx = (src_row + x) * 4;
            int dst_idx = (dst_row + start_x + x) * 4;
            // 复制RGBA四个通道
            output_data[dst_idx + 0] = scaled_data[src_idx + 0]; // R
            output_data[dst_idx + 1] = scaled_data[src_idx + 1]; // G
            output_data[dst_idx + 2] = scaled_data[src_idx + 2]; // B
            output_data[dst_idx + 3] = scaled_data[src_idx + 3]; // A
        }
    }

    // 释放临时数据
    free(scaled_data);
    
    *output_size = output_width * output_height * 4;
    return output_data;
}

// 拉伸模式：拉伸图像填满整个区域
static unsigned char* scale_stretch(const unsigned char* input_data,
                                    int input_width, int input_height,
                                    int output_width, int output_height,
                                    int* output_size) {
    printf("Scaling image to stretch: %dx%d -> %dx%d\n", 
           input_width, input_height, output_width, output_height);
    
    // 直接缩放到目标尺寸
    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        fprintf(stderr, "Failed to allocate memory for stretched image\n");
        return NULL;
    }

    // 使用stb_image_resize进行拉伸缩放
    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              output_data, output_width, output_height, 0,
                              STBIR_4CHANNEL);
    printf("Image stretched to: %dx%d\n", output_width, output_height);

    *output_size = output_width * output_height * 4;
    return output_data;
}

// 主缩放函数
unsigned char* scale_image(const unsigned char* input_data, 
                          int input_width, int input_height, 
                          int output_width, int output_height,
                          ScaleMode mode,
                          int* output_size) {
    switch (mode) {
        case SCALE_MODE_FILL:
            return scale_fill(input_data, input_width, input_height, 
                             output_width, output_height, output_size);
        case SCALE_MODE_FIT:
            return scale_fit(input_data, input_width, input_height, 
                            output_width, output_height, output_size);
        case SCALE_MODE_STRETCH:
            return scale_stretch(input_data, input_width, input_height, 
                                output_width, output_height, output_size);
        default:
            fprintf(stderr, "Unknown scale mode: %d\n", mode);
            return NULL;
    }
}
