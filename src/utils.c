#include "utils.h"

uint32_t hash(uint32_t key, int capacity) {
    return key % capacity;
}

uint32_t str_to_num(const char* str) {
    return (uintptr_t)str;
}

int hash_map_put(HashMap* hmi, uint32_t key, void* value) {
    
    if (hmi->count >= hmi->capacity) {
        Linked_List** old_items = hmi->items;
        int old_capacity = hmi->capacity;
        
        if (old_capacity == 0) hmi->capacity = 8;
        else hmi->capacity *= 2;
        hmi->items = calloc(hmi->capacity, sizeof(Linked_List*));

        if(!hmi->items) {
            perror("realloc error");
            exit(EXIT_FAILURE);
        }

        for (int i=0; i<old_capacity; ++i) {
            Linked_List* node = old_items[i];
            while (node) {
                Linked_List* next = node->next;
                int index = hash(node->key, hmi->capacity);
                node->next = hmi->items[index];
                hmi->items[index] = node;
                node = next;
            }
        }

        free(old_items);
    }

    int index = hash(key, hmi->capacity);
    
    Linked_List* node = hmi->items[index];
    while (node) {
        if (node->key == key) {
            node->value = value;
            return 0;
        }
        node = node->next;
    }
    
    Linked_List* new_node = malloc(sizeof(Linked_List));

    if (!new_node) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    new_node->key = key;
    new_node->value = value;
    new_node->next = hmi->items[index];
    hmi->items[index] = new_node;
    hmi->count++;
    
    return 0;
}

void* hash_map_get(HashMap* hmi, uint32_t key) {
    int index = hash(key, hmi->capacity);
    Linked_List* node = hmi->items[index];
    while (node) {
        if (node->key == key) {
            return node->value;
        }
        node = node->next;
    }
    return 0;
}

void free_hash_map(HashMap* hmi) {
    for (int i=0; i<hmi->capacity; ++i) {
        Linked_List* node = hmi->items[i];
        while (node) {
            Linked_List* next = node->next;
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(hmi->items);
    hmi->items = NULL;
    hmi->count = 0;
    hmi->capacity = 0;
}