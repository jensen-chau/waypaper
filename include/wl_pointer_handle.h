#ifndef WL_POINTER_HANDLE_H
#define WL_POINTER_HANDLE_H
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-client.h>
#include "context.h"

static void pointer_handle_enter(void* data, struct wl_pointer* pointer,
                                 uint32_t serial, struct wl_surface* surface,
                                 wl_fixed_t surface_x, wl_fixed_t surface_y) {
    printf("pointer enter\n");
}

static void pointer_handle_leave(void* data, struct wl_pointer* pointer,
                                 uint32_t serial, struct wl_surface* surface) {
    printf("pointer leave\n");
}

static void pointer_handle_motion(void* data, struct wl_pointer* pointer,
                                  uint32_t time, wl_fixed_t surface_x,
                                  wl_fixed_t surface_y) {
    printf("pointer motion,x:%.2f,y:%.2f\n", 
           wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
    Point* p = (Point*)malloc(sizeof(Point));
    *p = (Point){wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y)};
    handle_event(MOTION, p);
    free(p);
}

static void pointer_handle_button(void* data, struct wl_pointer* pointer,
                                  uint32_t serial, uint32_t time,
                                  uint32_t button, uint32_t state) {
    printf("pointer button,serial:%u,time:%u,button:%u,state:%s\n", 
           serial, time, button, state == WL_POINTER_BUTTON_STATE_PRESSED ? "PRESSED" : "RELEASED");
    handle_event(MOUSE_CLICK, 0);
}

static void pointer_handle_axis(void* data, struct wl_pointer* pointer,
                                uint32_t time, uint32_t axis,
                                wl_fixed_t value) {
}

static void pointer_handle_frame(void* data, struct wl_pointer* pointer) {
}

static void pointer_handle_axis_source(void* data, struct wl_pointer* pointer,
                                       uint32_t axis_source) {
}

static void pointer_handle_axis_stop(void* data, struct wl_pointer* pointer,
                                     uint32_t time, uint32_t axis) {
}

static void pointer_handle_axis_discrete(void* data, struct wl_pointer* pointer,
                                         uint32_t axis, int32_t discrete) {
}

static void pointer_handle_axis_relative_direction(void* data,
                                                   struct wl_pointer* pointer,
                                                   uint32_t axis,
                                                   uint32_t direction) {
}

static void pointer_handle_axis_value120(void* data,
                                         struct wl_pointer* wl_pointer,
                                         uint32_t axis, int32_t value120) {
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
    .axis_relative_direction = pointer_handle_axis_relative_direction,
    .axis_value120 = pointer_handle_axis_value120};

#endif
