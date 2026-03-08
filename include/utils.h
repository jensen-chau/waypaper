#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define LOG(fmt, ...) \
    do { \
        printf("[LOG] " fmt, ##__VA_ARGS__); \
    } while (0)

#define ERR(fmt, ...) \
    do { \
        fprintf(stderr, "[ERROR] " fmt, ##__VA_ARGS__); \
    } while (0)

typedef struct Linked_List Linked_List;

struct Linked_List {
    uint32_t key;
    void* value;
    struct Linked_List* next;
};

typedef struct {
    Linked_List** items;
    int count;
    int capacity;
} HashMap;

uint32_t hash(uint32_t key, int capacity);
uint32_t str_to_num(const char* str);
int hash_map_put(HashMap* hmi, uint32_t key, void* value);
void* hash_map_get(HashMap* hmi, uint32_t key);
void free_hash_map(HashMap* hmi);

#endif
