#ifndef WAYLAND_CONTEXT_H
#define WAYLAND_CONTEXT_H

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "linux-dmabuf-v1-protocol.h"
#include "viewporter-protocol.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"

#define MAX_OUTPUTS 10

typedef struct ClientState {
    struct wl_surface *focused_surface;
    struct xkb_context *context;
    
} ClientState;

typedef struct OutputInfo {
    struct wl_output *output;
    int32_t id;
    int32_t width;
    int32_t height;
    int32_t scale;
    int32_t x, y;  // 输出位置
    struct wl_surface *surface;
    struct zwlr_layer_surface_v1 *layer_surface;
    void* shm_data;
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    struct wl_callback *frame_callback;
    int configured;
} OutputInfo;

struct WaylandContext {
    struct wl_display *display;
    struct wl_shell *shell;
    struct wl_shm *shm;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    struct zwlr_layer_shell_v1 *layer_shell;
    struct wp_viewporter *viewporter;
    struct zwp_linux_dmabuf_v1 *dmabuf;
    struct ClientState *client_state;

    OutputInfo outputs[MAX_OUTPUTS];
    int num_outputs;
    int width;  // 默认宽度
    int height; // 默认高度
    
};

struct WaylandContext *wayland_context_init(int width, int height);

void wayland_context_cleanup(struct WaylandContext *ctx);

void create_pool_for_output(struct WaylandContext *ctx, int output_idx, int width, int height, int channels);

void create_surface_for_output(struct WaylandContext *ctx, int output_idx, int32_t width, int32_t height);

void recreate_callback(OutputInfo *output_info);

#endif
