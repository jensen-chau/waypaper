#ifndef WAYLAND_CONTEXT_H
#define WAYLAND_CONTEXT_H

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"

typedef struct ClientState {
    struct wl_keyboard *keyboard;
    struct wl_surface *focused_surface;
    
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *xkb_state;
    
    int ctrl_pressed;
    int shift_pressed;
    int alt_pressed;
    
    int32_t repeat_rate;
    int32_t repeat_delay;
} ClientState;

struct WaylandContext {
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_shell *shell;
    struct wl_shm *shm;
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_layer_surface_v1 *layer_surface;
    struct ClientState *client_state;

    void* shm_data;
    int width;
    int height;
    int configured;
};

struct WaylandContext *wayland_context_init(int width, int height);

void wayland_context_cleanup(struct WaylandContext *ctx);

#endif
