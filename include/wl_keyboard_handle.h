#ifndef WL_KEYBOARD_HANDLE_H
#define WL_KEYBOARD_HANDLE_H

#include <stdio.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>
#include <unistd.h>
#include "wayland_context.h"
#include <stdlib.h>

void keymap_handler(void* data, struct wl_keyboard* keyboard, uint32_t format,
               int32_t fd, uint32_t size){
    printf("keymap_handler\n");
    
    struct WaylandContext *ctx = (struct WaylandContext *)data;
    if (ctx->client_state) {
        // 如果已经存在，重置现有结构体
        ClientState *state = ctx->client_state;
        if (state->context) xkb_context_unref(state->context);
        if (state->keymap) xkb_keymap_unref(state->keymap);
        if (state->xkb_state) xkb_state_unref(state->xkb_state);
    } else {
        // 如果不存在，分配新的结构体
        ClientState *state = (struct ClientState *)malloc(sizeof(ClientState));
        ctx->client_state = state;
    }
    ClientState *state = ctx->client_state;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    char *map_str = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }
    
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_string(
        context, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS
    );
    
    struct xkb_state *xkb_state = xkb_state_new(keymap);
    
    state->keymap = keymap;
    state->xkb_state = xkb_state;
    state->context = context;
    state->focused_surface = ctx->surface;
    state->keyboard = keyboard;

    munmap(map_str, size);
    close(fd);
}

void enter_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
              struct wl_surface* surface, struct wl_array* keys) {
    printf("enter_handler: surface=%p\n", (void*)surface);
    
    struct WaylandContext *ctx = (struct WaylandContext *)data;
    if (ctx && ctx->client_state) {
        ctx->client_state->focused_surface = surface;
        printf("Focus set to surface: %p\n", (void*)surface);
    }
}

void leave_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
              struct wl_surface* surface) {}

void key_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
            uint32_t time, uint32_t key, uint32_t state);

void modifiers_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial,
                  uint32_t mods_depressed, uint32_t mods_latched,
                  uint32_t mods_locked, uint32_t group) {}

void repeat_info_handler(void* data, struct wl_keyboard* keyboard, int32_t rate,
                    int32_t delay) {}

struct wl_keyboard_listener keyboard_listener = {
    .keymap = keymap_handler,
    .enter = enter_handler,
    .key = key_handler,
    .leave = leave_handler,
    .modifiers = modifiers_handler,
    .repeat_info = repeat_info_handler};

#endif
