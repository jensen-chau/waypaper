#include "scale.h"
#include "stb_image_resize2.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char* scale_fill(const unsigned char* input_data,
                                 int input_width, int input_height,
                                 int output_width, int output_height,
                                 int* output_size) {
    float img_aspect = (float)input_width / input_height;
    float output_aspect = (float)output_width / output_height;
    
    int scale_width, scale_height;
    if (img_aspect > output_aspect) {
        scale_height = output_height;
        scale_width = (int)(scale_height * img_aspect);
    } else {
        scale_width = output_width;
        scale_height = (int)(scale_width / img_aspect);
    }
    
    
    unsigned char* scaled_data = malloc(scale_width * scale_height * 4);
    if (!scaled_data) {
        return NULL;
    }

    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              scaled_data, scale_width, scale_height, 0,
                              STBIR_4CHANNEL);

    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        free(scaled_data);
        return NULL;
    }
    
    int crop_start_x = (scale_width - output_width) / 2;
    int crop_start_y = (scale_height - output_height) / 2;
    
    
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
    

    free(scaled_data);
    
    *output_size = output_width * output_height * 4;
    return output_data;
}

static unsigned char* scale_fit(const unsigned char* input_data,
                                int input_width, int input_height,
                                int output_width, int output_height,
                                int* output_size) {
    float img_aspect = (float)input_width / input_height;
    float output_aspect = (float)output_width / output_height;
    
    int scale_width, scale_height;
    if (img_aspect > output_aspect) {
        scale_width = output_width;
        scale_height = (int)(scale_width / img_aspect);
    } else {
        scale_height = output_height;
        scale_width = (int)(scale_height * img_aspect);
    }
    
    
    unsigned char* scaled_data = malloc(scale_width * scale_height * 4);
    if (!scaled_data) {
        return NULL;
    }

    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              scaled_data, scale_width, scale_height, 0,
                              STBIR_4CHANNEL);

    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        free(scaled_data);
        return NULL;
    }
    
    for (int i = 0; i < output_width * output_height * 4; i += 4) {
        output_data[i + 0] = 0; // R
        output_data[i + 1] = 0; // G
        output_data[i + 2] = 0; // B
        output_data[i + 3] = 255; // A
    }
    
    int start_x = (output_width - scale_width) / 2;
    int start_y = (output_height - scale_height) / 2;
    
    for (int y = 0; y < scale_height; y++) {
        int src_row = y * scale_width;
        int dst_row = (start_y + y) * output_width;
        for (int x = 0; x < scale_width; x++) {
            int src_idx = (src_row + x) * 4;
            int dst_idx = (dst_row + start_x + x) * 4;
            output_data[dst_idx + 0] = scaled_data[src_idx + 0]; // R
            output_data[dst_idx + 1] = scaled_data[src_idx + 1]; // G
            output_data[dst_idx + 2] = scaled_data[src_idx + 2]; // B
            output_data[dst_idx + 3] = scaled_data[src_idx + 3]; // A
        }
    }

    free(scaled_data);
    
    *output_size = output_width * output_height * 4;
    return output_data;
}

static unsigned char* scale_stretch(const unsigned char* input_data,
                                    int input_width, int input_height,
                                    int output_width, int output_height,
                                    int* output_size) {
    
    unsigned char* output_data = malloc(output_width * output_height * 4);
    if (!output_data) {
        return NULL;
    }

    stbir_resize_uint8_linear(input_data, input_width, input_height, 0,
                              output_data, output_width, output_height, 0,
                              STBIR_4CHANNEL);

    *output_size = output_width * output_height * 4;
    return output_data;
}

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
            return NULL;
    }
}
