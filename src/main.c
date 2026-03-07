#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include "context.h"
#include "wayland_context.h"


int main() {
    printf("Initializing context...\n");
    struct Context* context = get_context(1920, 1080); // 使用默认分辨率

    if (!context) {
        fprintf(stderr, "Failed to initialize context\n");
        return 1;
    }

    struct WaylandContext* wayland_ctx = context->wayland_context;
    if (!wayland_ctx) {
        fprintf(stderr, "Failed to get wayland context\n");
        return 1;
    }

    run();

    printf("Wayland context created successfully with %d outputs\n", wayland_ctx->num_outputs);

    return 0;
}
