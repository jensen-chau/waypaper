#include "wayland_context.h"

#include "utils.h"
#include <endian.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include "linux-dmabuf-v1-protocol.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg-shell-protocol.h"
#include "wl_pointer_handle.h"
#include "viewporter-protocol.h"

static void output_geometry(void *data, struct wl_output *wl_output,
                           int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
                           int32_t subpixel, const char *make, const char *model,
                           int32_t transform) {
    OutputInfo *output = (OutputInfo*)data;
    output->x = x;
    output->y = y;
}

static void output_mode(void *data, struct wl_output *wl_output,
                       uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        OutputInfo *output = (OutputInfo*)data;
        output->width = width;
        output->height = height;
    }
}

static void output_done(void *data, struct wl_output *wl_output) {
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t scale) {
    OutputInfo *output = (OutputInfo*)data;
    output->scale = scale;
}

static const struct wl_output_listener output_listener = {
    .geometry = output_geometry,
    .mode = output_mode,
    .done = output_done,
    .scale = output_scale,
};

static void layer_surface_configure(void* data,
                                    struct zwlr_layer_surface_v1* surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
    OutputInfo* output_info = (OutputInfo*)data;
    if (!output_info) {
        return;
    }

    zwlr_layer_surface_v1_ack_configure(surface, serial);
    output_info->configured = 1;
    output_info->width = width;
    output_info->height = height;
}

static void layer_surface_closed(void* data,
                                 struct zwlr_layer_surface_v1* surface) {
}

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

void create_surface_for_output(struct WaylandContext* ctx, int output_idx, int32_t width, int32_t height) {
    OutputInfo* output_info = &ctx->outputs[output_idx];
    
    output_info->surface = wl_compositor_create_surface(ctx->compositor);
    if (!output_info->surface) {
        return;
    }

    output_info->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        ctx->layer_shell, output_info->surface, output_info->output, 
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "waypaper");
    if (!output_info->layer_surface) {
        return;
    }

    zwlr_layer_surface_v1_set_size(output_info->layer_surface, width, height);
    zwlr_layer_surface_v1_set_anchor(
        output_info->layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | 
        ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(output_info->layer_surface, -1);

    zwlr_layer_surface_v1_add_listener(output_info->layer_surface,
                                       &layer_surface_listener, output_info);

    wl_surface_commit(output_info->surface);

    
    while (!output_info->configured) {
        wl_display_roundtrip(ctx->display);
    }
}

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    if (!interface) return;
    if (!registry) {
        return;
    }

    struct WaylandContext* ctx = (struct WaylandContext*)data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        ctx->compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, version);
    } else if (strcmp(xdg_wm_base_interface.name, interface) == 0) {
        ctx->shell =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
    } else if (strcmp(wl_shm_interface.name, interface) == 0) {
        ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
    } else if (strcmp(zwlr_layer_shell_v1_interface.name, interface) == 0) {
        ctx->layer_shell = wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, version);
    } else if (strcmp(wl_seat_interface.name, interface) == 0) {
        ctx->seat =
            wl_registry_bind(registry, name, &wl_seat_interface, version);
    } else if (strcmp(wl_output_interface.name, interface) == 0) {
        // 处理输出设备
        if (ctx->num_outputs < MAX_OUTPUTS) {
            OutputInfo* output_info = &ctx->outputs[ctx->num_outputs];
            output_info->id = name;
            output_info->output = wl_registry_bind(registry, name, &wl_output_interface, 2);
            output_info->configured = 0;
            output_info->width = ctx->width;  
            output_info->height = ctx->height; 
            output_info->scale = 1;
            
            wl_output_add_listener(output_info->output, &output_listener, output_info);
            
            ctx->num_outputs++;
        }
    } else if (strcmp(wp_viewporter_interface.name, interface) == 0) {
        ctx->viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, version);
    } else if (strcmp(zwp_linux_dmabuf_v1_interface.name, interface) == 0) {
        ctx->dmabuf = wl_registry_bind(registry, name, &zwp_linux_dmabuf_v1_interface, version);
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    LOG("on_global_remove: %d\n", name);
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

void create_pool_for_output(struct WaylandContext* ctx, int output_idx, int width, int height, int channels) {
    LOG("Creating pool for output %d: %dx%d\n", output_idx, width, height);
    OutputInfo* output_info = &ctx->outputs[output_idx];
    
    char tmp_name[] = "/tmp/wayland-shm-XXXXXX";
    int fd = syscall(SYS_memfd_create, tmp_name, MFD_CLOEXEC);
    if (fd < 0) {
        ERR("Failed to create temporary file for output %d", output_idx);
        return;
    }

    int stride = channels * width;
    int size = width * height * channels; 
    if (ftruncate(fd, size) < 0) {
        ERR("Failed to truncate file for output %d", output_idx);
        close(fd);
        return;
    }

    output_info->shm_data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (output_info->shm_data == MAP_FAILED) {
        ERR("Failed to mmap file for output %d", output_idx);
        close(fd);
        return;
    }

    output_info->pool = wl_shm_create_pool(ctx->shm, fd, size);
    if (!output_info->pool) {
        ERR("Failed to create shm pool for output %d", output_idx);
        munmap(output_info->shm_data, size);
        close(fd);
        return;
    }

    output_info->buffer = wl_shm_pool_create_buffer(
        output_info->pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    if (!output_info->buffer) {
        ERR("Failed to create buffer for output %d", output_idx);
        wl_shm_pool_destroy(output_info->pool);
        munmap(output_info->shm_data, size);
        close(fd);
        return;
    }

    LOG("Pool created successfully for output %d\n", output_idx);
}

struct WaylandContext* wayland_context_init(int width, int height) {
    struct wl_display* display = wl_display_connect(0);

    if (!display) {
        return NULL;
    }

    struct wl_registry* registry = wl_display_get_registry(display);
    if (!registry) {
        wl_display_disconnect(display);
        return NULL;
    }

    struct WaylandContext* ctx = malloc(sizeof(struct WaylandContext));
    if (!ctx) {
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return NULL;
    }

    memset(ctx, 0, sizeof(struct WaylandContext));
    ctx->display = display;
    ctx->width = width;
    ctx->height = height;
    ctx->registry = registry;
    ctx->num_outputs = 0;

    wl_registry_add_listener(registry, &wayland_registry_listener, ctx);

    wl_display_roundtrip(display);
    wl_display_roundtrip(display);

    wl_display_roundtrip(display);

    if (!ctx->seat) {
        free(ctx);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return NULL;
    }

    ctx->pointer = wl_seat_get_pointer(ctx->seat);
    wl_pointer_add_listener(ctx->pointer, &pointer_listener, ctx);

    wl_display_roundtrip(display);

    for (int i = 0; i < ctx->num_outputs; i++) {
        OutputInfo* output_info = &ctx->outputs[i];
        create_surface_for_output(ctx, i, output_info->width, output_info->height);
    }

    return ctx;
}

void create_pool(struct WaylandContext* ctx, int width, int height, int channels) {
    if (ctx->num_outputs > 0) {
        create_pool_for_output(ctx, 0, width, height, channels);
    } else {
        ctx->num_outputs = 1;
        create_pool_for_output(ctx, 0, width, height, channels);
    }
}

void wayland_context_cleanup(struct WaylandContext* ctx) {
    if (!ctx) return;

    LOG("Cleaning up Wayland context...\n");

    for (int i = 0; i < ctx->num_outputs; i++) {
        OutputInfo* output_info = &ctx->outputs[i];
        
        if (output_info->layer_surface) {
            zwlr_layer_surface_v1_destroy(output_info->layer_surface);
        }

        if (output_info->surface) {
            wl_surface_destroy(output_info->surface);
        }

        if (output_info->pool) {
            wl_shm_pool_destroy(output_info->pool);
        }

        if (output_info->buffer) {
            wl_buffer_destroy(output_info->buffer);
        }

        if (output_info->shm_data) {
            munmap(output_info->shm_data, output_info->width * output_info->height * 4);
        }
        
        if (output_info->output) {
            wl_output_destroy(output_info->output);
        }
    }

    if (ctx->compositor) {
        wl_compositor_destroy(ctx->compositor);
    }

    if (ctx->shm) {
        wl_shm_destroy(ctx->shm);
    }

    if (ctx->layer_shell) {
        zwlr_layer_shell_v1_destroy(ctx->layer_shell);
    }

    if (ctx->shell) {
        wl_shell_destroy(ctx->shell);
    }

    if (ctx->registry) {
        wl_registry_destroy(ctx->registry);
    }

    if (ctx->display) {
        wl_display_disconnect(ctx->display);
    }

    free(ctx);
}


