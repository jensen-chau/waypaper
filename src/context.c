#include <wayland-client-protocol.h>
#define STB_IMAGE_IMPLEMENTATION
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "context.h"
#include "scale.h"
#include "stb_image.h"
#include "utils.h"
#include "wayland_context.h"

struct Node;

static int should_exit = 0;
static struct Context* ctx = NULL;
static int is_update = 1;

struct Context* get_context(int width, int height) {
    if (ctx != NULL) {
        return ctx;
    }
    ctx = malloc(sizeof(struct Context));
    if (!ctx) {
        ERR("Failed to allocate context\n");
        return NULL;
    }

    ctx->wayland_context = wayland_context_init(width, height);
    if (!ctx->wayland_context) {
        ERR("Failed to initialize wayland context\n");
        free(ctx);
        ctx = NULL;
        return NULL;
    }
    return ctx;
}

void app_exit() {
    should_exit = 1;
}

int load_wallpaper(const char* path) {
    int width, height;
    int channels_in_file;
    unsigned char* img_data =
        stbi_load(path, &width, &height, &channels_in_file, 4);
    if (img_data == NULL) {
        ERR("Failed to load image: %s\n", stbi_failure_reason());
        return -1;
    }
    LOG("Image loaded: %dx%d, channels:%d\n", width, height, channels_in_file);

    struct WaylandContext* wayland_ctx = ctx->wayland_context;

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
        OutputInfo* first_output = &wayland_ctx->outputs[0];
        int output_width = first_output->width;
        int output_height = first_output->height;

        LOG("All outputs have same resolution: %dx%d, optimizing...\n",
            output_width, output_height);

        int output_size;
        unsigned char* resized_data =
            scale_image(img_data, width, height, output_width, output_height,
                        SCALE_MODE_FILL, &output_size);
        if (!resized_data) {
            ERR("Failed to scale image\n");
            stbi_image_free(img_data);
            return -1;
        }

        for (int j = 0; j < output_width * output_height; j++) {
            unsigned char r = resized_data[j * 4 + 0];
            unsigned char b = resized_data[j * 4 + 2];
            resized_data[j * 4 + 0] = b;
            resized_data[j * 4 + 2] = r;
        }

        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];

            LOG("Before create_pool_for_output for output %d, old buffer: %p\n",
                i, output_info->buffer);

            create_pool_for_output(wayland_ctx, i, output_width, output_height,
                                   4);

            LOG("After create_pool_for_output for output %d, new buffer: %p\n",
                i, output_info->buffer);

            if (output_info->shm_data) {
                memcpy(output_info->shm_data, resized_data,
                       output_width * output_height * 4);

                int size = output_width * output_height * 4;
                if (msync(output_info->shm_data, size, MS_SYNC) != 0) {
                    ERR("msync failed: %s", strerror(1));
                }

                LOG("Attaching buffer %p to surface for output %d\n",
                    output_info->buffer, i);
                wl_surface_attach(output_info->surface, output_info->buffer, 0,
                                  0);
                wl_surface_damage_buffer(output_info->surface, 0, 0,
                                         output_info->width,
                                         output_info->height);
                LOG("Committing surface for output %d\n", i);
                wl_surface_commit(output_info->surface);
                LOG("Surface committed for output %d\n", i);
            }
        }

        LOG("Flushing display after loading wallpaper (same resolution)\n");
        wl_display_flush(wayland_ctx->display);

        free(resized_data);
    } else {
        for (int i = 0; i < wayland_ctx->num_outputs; i++) {
            OutputInfo* output_info = &wayland_ctx->outputs[i];
            int output_width = output_info->width;
            int output_height = output_info->height;

            LOG("Processing output %d: %dx%d\n", i, output_width,
                output_height);

            int output_size;
            unsigned char* resized_data =
                scale_image(img_data, width, height, output_width,
                            output_height, SCALE_MODE_FILL, &output_size);
            if (!resized_data) {
                ERR("Failed to scale image for output %d\n", i);
                continue;
            }

            for (int j = 0; j < output_width * output_height; j++) {
                unsigned char r = resized_data[j * 4 + 0];
                unsigned char b = resized_data[j * 4 + 2];
                resized_data[j * 4 + 0] = b;
                resized_data[j * 4 + 2] = r;
            }

            LOG("Before create_pool_for_output for output %d, old buffer: %p\n",
                i, output_info->buffer);
            create_pool_for_output(wayland_ctx, i, output_width, output_height,
                                   4);
            LOG("After create_pool_for_output for output %d, new buffer: %p\n",
                i, output_info->buffer);

            if (output_info->shm_data) {
                memcpy(output_info->shm_data, resized_data,
                       output_width * output_height * 4);
                int size = output_width * output_height * 4;
                if (msync(output_info->shm_data, size, MS_SYNC) != 0) {
                    ERR("msync failed: %s", strerror(1));
                }
                LOG("Attaching buffer %p to surface for output %d\n",
                    output_info->buffer, i);
                wl_surface_attach(output_info->surface, output_info->buffer, 0,
                                  0);
                wl_surface_damage_buffer(output_info->surface, 0, 0,
                                         output_info->width,
                                         output_info->height);
                LOG("Committing surface for output %d\n", i);
                wl_surface_commit(output_info->surface);
                LOG("Surface committed for output %d\n", i);
            }

            free(resized_data);
        }

        LOG("Flushing display after loading wallpaper (different resolution)\n");
        wl_display_flush(wayland_ctx->display);
    }

    is_update = 1;

    stbi_image_free(img_data);
    return 0;
}

void run() {
    if (!ctx) {
        ERR("Error: context is NULL\n");
        return;
    }

    struct WaylandContext* wayland_ctx = ctx->wayland_context;
    if (!wayland_ctx) {
        ERR("Error: wayland context is NULL\n");
        return;
    }

    while (!should_exit) {
        if (wl_display_dispatch(wayland_ctx->display) == -1) {
            break;
        }

        LOG("wayland event processed\n");
    }

    wayland_context_cleanup(wayland_ctx);
    free(ctx);
    ctx = NULL;
}

void handle_event(PointEvent point_event, void* data) {
    LOG("point_event:%d\n", point_event);
}

Point get_mouse_point_pos() {
    return ctx->mouse_pos;
}
