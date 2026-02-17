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
#include "wl_keyboard_handle.h"

void create_pool(struct WaylandContext* ctx);

void create_surface(struct WaylandContext* ctx);

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    if (!interface) return;
    if (!registry) {
        fprintf(stderr, "on_global: registry is NULL!\n");
        return;
    }

    //printf("name:%d,interface:%s,version:%d\n", name, interface, version);

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
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    printf("on_global_remove: %d\n", name);
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

static void layer_surface_configure(void* data,
                                    struct zwlr_layer_surface_v1* surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
    struct WaylandContext* ctx = (struct WaylandContext*)data;
    if (!ctx) {
        fprintf(stderr, "Error: ctx is NULL in layer_surface_configure\n");
        return;
    }

    zwlr_layer_surface_v1_ack_configure(surface, serial);
    ctx->configured = 1;
    printf("Layer surface configured: %dx%d\n", width, height);
}

static void layer_surface_closed(void* data,
                                 struct zwlr_layer_surface_v1* surface) {
}

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};


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

    printf("Adding registry listener...\n");
    wl_registry_add_listener(registry, &wayland_registry_listener, ctx);

    printf("Dispatching events...\n");
    int ret = wl_display_roundtrip(display);
    if (ret < 0) {
        fprintf(stderr, "Failed to dispatch events: %d\n", ret);
        free(ctx);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return NULL;
    }

    fflush(stdout);

    ctx->display = display;
    ctx->width = width;
    ctx->height = height;
    ctx->registry = registry;
    ctx->configured = 0;

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

    printf("Creating pool...\n");
    create_pool(ctx);
    printf("Creating surface...\n");
    create_surface(ctx);

    ctx->keyboard = wl_seat_get_keyboard(ctx->seat);
    wl_keyboard_add_listener(ctx->keyboard, &keyboard_listener, ctx);

    printf("Wayland context initialized successfully\n");
    return ctx;
}

void create_pool(struct WaylandContext* ctx) {
    printf("Creating pool: %dx%d\n", ctx->width, ctx->height);
    char tmp_name[] = "/tmp/wayland-shm-XXXXXX";
    int fd = mkstemp(tmp_name);
    if (fd < 0) {
        fprintf(stderr, "Failed to create temporary file\n");
        return;
    }

    int stride = 4 * ctx->width;
    int size = ctx->width * ctx->height * stride;
    if (ftruncate(fd, size) < 0) {
        fprintf(stderr, "Failed to truncate file\n");
        close(fd);
        return;
    }

    ctx->shm_data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ctx->shm_data == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap file\n");
        close(fd);
        return;
    }

    ctx->pool = wl_shm_create_pool(ctx->shm, fd, size);
    if (!ctx->pool) {
        fprintf(stderr, "Failed to create shm pool\n");
        munmap(ctx->shm_data, size);
        close(fd);
        return;
    }

    ctx->buffer = wl_shm_pool_create_buffer(
        ctx->pool, 0, ctx->width, ctx->height, stride, WL_SHM_FORMAT_XRGB8888);
    if (!ctx->buffer) {
        fprintf(stderr, "Failed to create buffer\n");
        wl_shm_pool_destroy(ctx->pool);
        munmap(ctx->shm_data, size);
        close(fd);
        return;
    }

    unlink(tmp_name);
    printf("Pool created successfully\n");
}

void wayland_context_cleanup(struct WaylandContext* ctx) {
    if (!ctx) return;

    printf("Cleaning up Wayland context...\n");

    if (ctx->layer_surface) {
        zwlr_layer_surface_v1_destroy(ctx->layer_surface);
    }

    if (ctx->surface) {
        wl_surface_destroy(ctx->surface);
    }

    if (ctx->pool) {
        wl_shm_pool_destroy(ctx->pool);
    }

    if (ctx->buffer) {
        wl_buffer_destroy(ctx->buffer);
    }

    if (ctx->shm_data) {
        munmap(ctx->shm_data, ctx->width * ctx->height * 4);
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

void create_surface(struct WaylandContext* ctx) {
    printf("Creating surface...\n");
    ctx->surface = wl_compositor_create_surface(ctx->compositor);
    printf("Surface pointer: %p\n", (void*)ctx->surface);
    if (!ctx->surface) {
        fprintf(stderr, "Failed to create surface\n");
        return;
    }

    ctx->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        ctx->layer_shell, ctx->surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
        "game");
    printf("Layer surface pointer: %p\n", (void*)ctx->layer_surface);
    if (!ctx->layer_surface) {
        fprintf(stderr, "Failed to get layer surface\n");
        return;
    }

    zwlr_layer_surface_v1_set_size(ctx->layer_surface, ctx->width, ctx->height);
    zwlr_layer_surface_v1_set_anchor(
        ctx->layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(ctx->layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(ctx->layer_surface, 1);

    // 注册层表面监听器
    zwlr_layer_surface_v1_add_listener(ctx->layer_surface,
                                       &layer_surface_listener, ctx);

    wl_surface_commit(ctx->surface);
    printf("Surface committed\n");
    wl_display_roundtrip(ctx->display);

    // 等待层表面配置
    while (!ctx->configured) {
        wl_display_roundtrip(ctx->display);
    }
    printf("Surface created successfully\n");
}
