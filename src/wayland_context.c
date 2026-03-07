#include "wayland_context.h"

#include <endian.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg-shell-protocol.h"
#include "wl_pointer_handle.h"

static void output_geometry(void *data, struct wl_output *wl_output,
                           int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
                           int32_t subpixel, const char *make, const char *model,
                           int32_t transform) {
    printf("Output geometry: x=%d, y=%d, width=%d, height=%d\n", x, y, physical_width, physical_height);
}

static void output_mode(void *data, struct wl_output *wl_output,
                       uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    // 只处理当前模式
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        OutputInfo *output = (OutputInfo*)data;
        output->width = width;
        output->height = height;
        printf("Output mode: %dx%d (current)\n", width, height);
    }
}

static void output_done(void *data, struct wl_output *wl_output) {
    printf("Output done event\n");
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t scale) {
    OutputInfo *output = (OutputInfo*)data;
    output->scale = scale;
    printf("Output scale: %d\n", scale);
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
        fprintf(stderr, "Error: output_info is NULL in layer_surface_configure\n");
        return;
    }

    zwlr_layer_surface_v1_ack_configure(surface, serial);
    output_info->configured = 1;
    printf("Layer surface configured: %dx%d\n", width, height);
    output_info->width = width;
    output_info->height = height;
}

static void layer_surface_closed(void* data,
                                 struct zwlr_layer_surface_v1* surface) {
    printf("Layer surface closed\n");
}

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

void create_surface_for_output(struct WaylandContext* ctx, int output_idx, int32_t width, int32_t height) {
    printf("Creating surface for output %d: %dx%d\n", output_idx, width, height);
    
    OutputInfo* output_info = &ctx->outputs[output_idx];
    
    output_info->surface = wl_compositor_create_surface(ctx->compositor);
    if (!output_info->surface) {
        fprintf(stderr, "Failed to create surface for output %d\n", output_idx);
        return;
    }

    output_info->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        ctx->layer_shell, output_info->surface, output_info->output, 
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wallpaper");
    if (!output_info->layer_surface) {
        fprintf(stderr, "Failed to get layer surface for output %d\n", output_idx);
        return;
    }

    zwlr_layer_surface_v1_set_size(output_info->layer_surface, width, height);
    zwlr_layer_surface_v1_set_anchor(
        output_info->layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | 
        ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(output_info->layer_surface, -1);

    // 注册层表面监听器
    zwlr_layer_surface_v1_add_listener(output_info->layer_surface,
                                       &layer_surface_listener, output_info);

    wl_surface_commit(output_info->surface);
    printf("Surface committed for output %d\n", output_idx);
    
    // 等待层表面配置
    while (!output_info->configured) {
        wl_display_roundtrip(ctx->display);
    }
    printf("Surface created successfully for output %d\n", output_idx);
}

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    if (!interface) return;
    if (!registry) {
        fprintf(stderr, "on_global: registry is NULL!\n");
        return;
    }

    struct WaylandContext* ctx = (struct WaylandContext*)data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        printf("Binding compositor\n");
        ctx->compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, version);
        if (!ctx->compositor) {
            fprintf(stderr, "Failed to bind compositor\n");
        } else {
            printf("Compositor bound successfully\n");
        }
    } else if (strcmp(xdg_wm_base_interface.name, interface) == 0) {
        printf("Binding xdg_wm_base...\n");
        ctx->shell =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        if (!ctx->shell) {
            fprintf(stderr, "Failed to bind xdg_wm_base\n");
        } else {
            printf("Xdg wm base bound successfully\n");
        }
    } else if (strcmp(wl_shm_interface.name, interface) == 0) {
        printf("Binding wl_shm...\n");
        ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
        if (!ctx->shm) {
            fprintf(stderr, "Failed to bind wl_shm\n");
        } else {
            printf("Shm bound successfully\n");
        }
    } else if (strcmp(zwlr_layer_shell_v1_interface.name, interface) == 0) {
        printf("Binding zwlr_layer_shell_v1...\n");
        ctx->layer_shell = wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, version);
        if (!ctx->layer_shell) {
            fprintf(stderr, "Failed to bind zwlr_layer_shell_v1\n");
        } else {
            printf("Layer shell bound successfully\n");
        }
    } else if (strcmp(wl_seat_interface.name, interface) == 0) {
        printf("Binding wl_seat (version %d)...\n", version);
        ctx->seat =
            wl_registry_bind(registry, name, &wl_seat_interface, version);
        if (!ctx->seat) {
            fprintf(stderr, "Failed to bind wl_seat");
        } else {
            printf("wl_seat bind successfully (version %d)\n", version);
        }
    } else if (strcmp(wl_output_interface.name, interface) == 0) {
        // 处理输出设备
        if (ctx->num_outputs < MAX_OUTPUTS) {
            OutputInfo* output_info = &ctx->outputs[ctx->num_outputs];
            output_info->id = name;
            output_info->output = wl_registry_bind(registry, name, &wl_output_interface, 2);
            output_info->configured = 0;
            output_info->width = ctx->width;  // 默认值
            output_info->height = ctx->height; // 默认值
            output_info->scale = 1;
            
            wl_output_add_listener(output_info->output, &output_listener, output_info);
            printf("Bound output %d (id: %d)\n", ctx->num_outputs, name);
            
            ctx->num_outputs++;
        } else {
            printf("Maximum number of outputs reached\n");
        }
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    printf("on_global_remove: %d\n", name);
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

void create_pool_for_output(struct WaylandContext* ctx, int output_idx, int width, int height, int channels) {
    printf("Creating pool for output %d: %dx%d\n", output_idx, width, height);
    OutputInfo* output_info = &ctx->outputs[output_idx];
    
    char tmp_name[] = "/tmp/wayland-shm-XXXXXX";
    int fd = mkstemp(tmp_name);
    if (fd < 0) {
        fprintf(stderr, "Failed to create temporary file for output %d\n", output_idx);
        return;
    }

    int stride = channels * width;
    int size = width * height * channels; // 每像素4字节 (XRGB8888)
    if (ftruncate(fd, size) < 0) {
        fprintf(stderr, "Failed to truncate file for output %d\n", output_idx);
        close(fd);
        return;
    }

    output_info->shm_data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (output_info->shm_data == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap file for output %d\n", output_idx);
        close(fd);
        return;
    }

    output_info->pool = wl_shm_create_pool(ctx->shm, fd, size);
    if (!output_info->pool) {
        fprintf(stderr, "Failed to create shm pool for output %d\n", output_idx);
        munmap(output_info->shm_data, size);
        close(fd);
        return;
    }

    output_info->buffer = wl_shm_pool_create_buffer(
        output_info->pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    if (!output_info->buffer) {
        fprintf(stderr, "Failed to create buffer for output %d\n", output_idx);
        wl_shm_pool_destroy(output_info->pool);
        munmap(output_info->shm_data, size);
        close(fd);
        return;
    }

    unlink(tmp_name);
    printf("Pool created successfully for output %d\n", output_idx);
}

struct WaylandContext* wayland_context_init(int width, int height) {
    printf("Connecting to Wayland display...\n");
    struct wl_display* display = wl_display_connect(0);

    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return NULL;
    }
    printf("Connected to Wayland display\n");

    struct wl_registry* registry = wl_display_get_registry(display);
    printf("Registry pointer: %p\n", (void*)registry);
    if (!registry) {
        fprintf(stderr, "Failed to get Wayland registry\n");
        wl_display_disconnect(display);
        return NULL;
    }

    struct WaylandContext* ctx = malloc(sizeof(struct WaylandContext));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate WaylandContext\n");
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return NULL;
    }

    // 初始化输出数组
    memset(ctx, 0, sizeof(struct WaylandContext));
    ctx->display = display;
    ctx->width = width;
    ctx->height = height;
    ctx->registry = registry;
    ctx->num_outputs = 0;

    printf("Adding registry listener...\n");
    wl_registry_add_listener(registry, &wayland_registry_listener, ctx);

    printf("Dispatching events...\n");
    // 多次roundtrip以确保获取所有输出信息
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);

    // 确保所有服务都已绑定
    wl_display_roundtrip(display);

    printf("Seat pointer: %p\n", (void*)ctx->seat);
    if (!ctx->seat) {
        fprintf(stderr, "Failed to get seat\n");
        free(ctx);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return NULL;
    }

    ctx->pointer = wl_seat_get_pointer(ctx->seat);
    wl_pointer_add_listener(ctx->pointer, &pointer_listener, ctx);

    // 再次确保事件处理完成
    wl_display_roundtrip(display);

    // 为每个输出创建表面
    for (int i = 0; i < ctx->num_outputs; i++) {
        OutputInfo* output_info = &ctx->outputs[i];
        printf("Creating surface for output %d: %dx%d\n", i, output_info->width, output_info->height);
        create_surface_for_output(ctx, i, output_info->width, output_info->height);
    }

    printf("Wayland context initialized successfully with %d outputs\n", ctx->num_outputs);
    return ctx;
}

// 保留旧函数名以兼容现有代码，但调用新的多屏实现
void create_pool(struct WaylandContext* ctx, int width, int height, int channels) {
    // 为第一个输出创建池（向后兼容）
    if (ctx->num_outputs > 0) {
        create_pool_for_output(ctx, 0, width, height, channels);
    } else {
        // 如果没有检测到输出，创建默认输出
        ctx->num_outputs = 1;
        create_pool_for_output(ctx, 0, width, height, channels);
    }
}

void wayland_context_cleanup(struct WaylandContext* ctx) {
    if (!ctx) return;

    printf("Cleaning up Wayland context...\n");

    // 清理每个输出
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


