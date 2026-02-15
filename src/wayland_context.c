#include "wayland_context.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg-shell-protocol.h"

void create_pool(struct WaylandContext* ctx);

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    if (!interface) return;
    if (!registry) {
        fprintf(stderr, "on_global: registry is NULL!\n");
        return;
    }

    //printf("name:%d,interface:%s,version:%d\n", name, interface, version);

    struct WaylandContext* ctx = (struct WaylandContext*)data;

    if (strcmp(wl_shell_interface.name, interface) == 0) {
        printf("Binding wl_shell...\n");
        ctx->shell =
            wl_registry_bind(registry, name, &wl_shell_interface, version);
        if (!ctx->shell) {
            fprintf(stderr, "Failed to bind wl_shell\n");
        }
    } else if (strcmp(xdg_wm_base_interface.name, interface) == 0) {
        printf("Binding xdg_wm_base...\n");
        ctx->shell =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        if (!ctx->shell) {
            fprintf(stderr, "Failed to bind xdg_wm_base\n");
        }
    } else if (strcmp(wl_shm_interface.name, interface) == 0) {
        printf("Binding wl_shm...\n");
        ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
        if (!ctx->shm) {
            fprintf(stderr, "Failed to bind wl_shm\n");
        }
    } else if (strcmp(wl_surface_interface.name, interface) == 0) {
        printf("Binding wl_surface...\n");
        ctx->surface =
            wl_registry_bind(registry, name, &wl_surface_interface, version);
        if (!ctx->surface) {
            fprintf(stderr, "Failed to bind wl_surface\n");
        }
    } else if (strcmp(wl_compositor_interface.name, interface) == 0) {
        printf("Binding wl_compositor...\n");
        ctx->compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, version);
        if (!ctx->compositor) {
            fprintf(stderr, "Failed to bind wl_compositor\n");
        }
    } else if (strcmp(zwlr_layer_shell_v1_interface.name, interface) == 0) {
        printf("Binding zwlr_layer_shell_v1...\n");
        ctx->layer_shell = wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, version);
        if (!ctx->layer_shell) {
            fprintf(stderr, "Failed to bind zwlr_layer_shell_v1\n");
        }
    } else if (strcmp(wl_shm_pool_interface.name, interface) == 0) {
        printf("Binding wl_shm_pool...\n");
        ctx->pool = wl_registry_bind(registry, name, &wl_shm_pool_interface, version);
        if (!ctx->pool) {
            fprintf(stderr, "Failed to bind wl_shm_pool\n");
        }
    } else if (strcmp(wl_buffer_interface.name, interface) == 0) {
        printf("Binding wl_buffer...\n");
        ctx->buffer = wl_registry_bind(registry, name, &wl_buffer_interface, version);
        if (!ctx->buffer) {
            fprintf(stderr, "Failed to bind wl_buffer\n");
        }
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    printf("on_global_remove\n");
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

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

    create_pool(ctx);
    

    printf("Wayland context initialized successfully\n");
    return ctx;
}


void create_pool(struct WaylandContext* ctx) {
    char tmp_name[] = "/tmp/wayland-shm-xxx";
    int fd = mkstemp(tmp_name);
    if (fd < 0) {
        fprintf(stderr, "Failed to create temporary file\n");
        return;
    }

    int stride = 4;
    int size = ctx->width * ctx->height * stride;
    if (ftruncate(fd, size) < 0) {
        fprintf(stderr, "Failed to truncate file\n");
        return;
    }

    ctx->shm_data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ctx->shm_data == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap file\n");
        return;
    }
    
    ctx->pool = wl_shm_create_pool(ctx->shm, fd, size); 
    ctx->buffer = wl_shm_pool_create_buffer(ctx->pool, 0, ctx->width, ctx->height, stride, WL_SHM_FORMAT_XRGB8888);
    
    close(fd);
}

