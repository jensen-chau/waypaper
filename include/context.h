#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>



typedef struct {
    float x;
    float y;
} Point;


struct Context {
    Point mouse_pos;
};

#endif
