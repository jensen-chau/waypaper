#include <stdio.h>
#include <stdlib.h>
#include <wayland-client.h>

int main() {
    printf("Hello, World!\n");

    struct wl_display* display = wl_display_connect(0);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }
    
    printf("Connected to Wayland display\n");
    printf("display pointer: %p\n", (void*)display);
    
    wl_display_disconnect(display);
    return 0;
}