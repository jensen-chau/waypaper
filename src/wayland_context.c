#include "wayland_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client.h>

void on_global(void* data, struct wl_registry* registry, uint32_t name,
               const char* interface, uint32_t version) {
    printf("on_global: %s\n", interface);
    struct WaylandContext* ctx = (struct WaylandContext*)data;
    printf("dump\n");
    if (strcmp(wl_shell_interface.name, interface) == 0) {
        ctx->shell =
            wl_registry_bind(ctx->registry, name, &wl_shell_interface, version);
    } else if (strcmp(wl_shm_interface.name, interface) == 0) {
        ctx->shm =
            wl_registry_bind(ctx->registry, name, &wl_shm_interface, version);
    } else if (strcmp(wl_surface_interface.name, interface) == 0) {
        ctx->surface = wl_registry_bind(ctx->registry, name,
                                        &wl_surface_interface, version);
    }
}

void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
    printf("on_global_remove\n");
}

static struct wl_registry_listener wayland_registry_listener = {
    .global = on_global, .global_remove = on_global_remove};

struct WaylandContext* wayland_context_init() {
    struct wl_display* display = wl_display_connect(0);

    struct wl_registry* registry = wl_display_get_registry(display);

    struct WaylandContext* ctx = malloc(sizeof(struct WaylandContext));

    wl_registry_add_listener(registry, &wayland_registry_listener, ctx);

    wl_display_roundtrip(display);

    fflush(stdout);

    ctx->display = display;
    ctx->registry = registry;
    return ctx;
}
