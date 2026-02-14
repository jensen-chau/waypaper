#ifndef WAYLAND_CONTEXT_H
#define WAYLAND_CONTEXT_H

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"


struct WaylandContext {
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_shell *shell;
    struct wl_shm *shm;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct zwlr_layer_shell_v1 *layer_shell;
};

struct WaylandContext *wayland_context_init();

void wayland_context_cleanup(struct WaylandContext *ctx);

#endif
