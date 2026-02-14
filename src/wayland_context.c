#include "wayland_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client.h>

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    if (!interface) return;
    if (!registry) {
        fprintf(stderr, "on_global: registry is NULL!\n");
        return;
    }
    
    struct WaylandContext* ctx = (struct WaylandContext*)data;
    
    if (strcmp(wl_shell_interface.name, interface) == 0) {
        printf("Binding wl_shell...\n");
        ctx->shell = wl_registry_bind(registry, name, &wl_shell_interface, version);
        if (!ctx->shell) {
            fprintf(stderr, "Failed to bind wl_shell\n");
        }
    } else if (strcmp(wl_shm_interface.name, interface) == 0) {
        printf("Binding wl_shm...\n");
        ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
        if (!ctx->shm) {
            fprintf(stderr, "Failed to bind wl_shm\n");
        }
    } else if (strcmp(wl_surface_interface.name, interface) == 0) {
        printf("Binding wl_surface...\n");
        ctx->surface = wl_registry_bind(registry, name, &wl_surface_interface, version);
        if (!ctx->surface) {
            fprintf(stderr, "Failed to bind wl_surface\n");
        }
    } else if (strcmp(wl_compositor_interface.name, interface) == 0) {
        printf("Binding wl_compositor...\n");
        ctx->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
        if (!ctx->compositor) {
            fprintf(stderr, "Failed to bind wl_compositor\n");
        }
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    printf("on_global_remove\n");
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

struct WaylandContext* wayland_context_init() {
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
    ctx->registry = registry;
    printf("Wayland context initialized successfully\n");
    return ctx;
}
