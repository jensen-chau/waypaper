#ifndef SCALE_H
#define SCALE_H

#include <stdint.h>

// 缩放模式枚举
typedef enum {
    SCALE_MODE_FILL,     // 填充模式（按比例缩放再裁切）
    SCALE_MODE_FIT,      // 适应模式（完整显示图像，可能有黑边）
    SCALE_MODE_STRETCH   // 拉伸模式（拉伸图像填满整个区域）
} ScaleMode;

// 缩放图像函数
unsigned char* scale_image(const unsigned char* input_data, 
                          int input_width, int input_height, 
                          int output_width, int output_height,
                          ScaleMode mode,
                          int* output_size);

#endif