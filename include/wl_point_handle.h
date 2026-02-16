#ifndef WL_POINT_HANDLE_H
#define WL_POINT_HANDLE_H
#include <wayland-client.h>
#include <stdio.h>

static void pointer_handle_enter(void *data, struct wl_pointer *pointer,
                                  uint32_t serial, struct wl_surface *surface,
                                  wl_fixed_t surface_x, wl_fixed_t surface_y) {
    printf("pointer enter\n");
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer,
                                  uint32_t serial, struct wl_surface *surface) {
    printf("pointer leave\n");
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer,
                                   uint32_t time, wl_fixed_t surface_x,
                                   wl_fixed_t surface_y) {
    printf("pointer motion,x:%d,y:%d\n", surface_x, surface_y);
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer,
                                   uint32_t serial, uint32_t time,
                                   uint32_t button, uint32_t state) {
    printf("pointer button,x:%d,y:%d\n", serial, time);
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer,
                                 uint32_t time, uint32_t axis,
                                 wl_fixed_t value) {
}

static void pointer_handle_frame(void *data, struct wl_pointer *pointer) {
}

static void pointer_handle_axis_source(void *data, struct wl_pointer *pointer,
                                        uint32_t axis_source) {
}

static void pointer_handle_axis_stop(void *data, struct wl_pointer *pointer,
                                      uint32_t time, uint32_t axis) {
}

static void pointer_handle_axis_discrete(void *data, struct wl_pointer *pointer,
                                          uint32_t axis, int32_t discrete) {
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
    .frame = pointer_handle_frame,
    .axis_source = pointer_handle_axis_source,
    .axis_stop = pointer_handle_axis_stop,
    .axis_discrete = pointer_handle_axis_discrete,
};

#endif
