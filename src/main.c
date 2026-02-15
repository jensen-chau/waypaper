#include <stdio.h>
#include "wayland_context.h"

int main() {
    printf("Hello, World!\n");

    struct WaylandContext* ctx = wayland_context_init();
    if (!ctx) {
        fprintf(stderr, "Failed to initialize Wayland context\n");
        return 1;
    }
 
    printf("Wayland context created successfully\n");
    return 0;
}
