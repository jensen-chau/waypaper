#include <stdint.h>
#include <stdio.h>
#include <wayland-client-core.h>

#include "wayland_context.h"

int main() {
    printf("Hello, World!\n");

    struct WaylandContext* ctx = wayland_context_init(200, 200);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize Wayland context\n");
        return 1;
    }

    printf("Wayland context created successfully\n");
    
    while (1) {
        wl_display_dispatch_pending(ctx->display);
        for (int i = 0; i < 200 * 200; i++) {
            ((uint32_t*)(ctx->shm_data))[i] = 0xFF0000FF;
        }

        wl_surface_attach(ctx->surface, ctx->buffer, 0, 0);
        wl_surface_damage(ctx->surface, 0, 0, 200, 200);
        wl_surface_commit(ctx->surface);
    }
    
    wayland_context_cleanup(ctx);
    return 0;
}
