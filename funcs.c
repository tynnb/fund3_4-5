#include "funcs.h"

Heap create_heap(size_t initial_capacity) {
    Heap heap;
    heap.data = NULL;
    heap.size = 0;
    heap.capacity = 0;
    
    if (initial_capacity > 0) {
        heap.data = (int*)malloc(initial_capacity * sizeof(int));
        if (heap.data) {
            heap.capacity = initial_capacity;
        }
    }
    return heap;
}

void delete_heap(Heap *h) {
    if (!h) {
        return;
    }

    if (h->data) {
        free(h->data);
        h->data = NULL;
    }
    h->size = 0;
    h->capacity = 0;
}

int is_empty_heap(const Heap *h) {
    return !h || h->size == 0;
}

size_t size_heap(const Heap *h) {
    if (h == NULL) {
        return 0;
    }
    return h->size;
}

int peek_heap(const Heap *h) {
    if (!h || is_empty_heap(h)) {
        return -1;
    }
    return h->data[0];
}

void push_heap(Heap *h, int value) {
    if (!h) {
        return;
    }
    
    if (h->size >= h->capacity) {
        size_t new_capacity;
        if (h->capacity == 0) {
            new_capacity = 1;
        } else {
            new_capacity = h->capacity * 2;
        }
        int *new_data = (int*)realloc(h->data, new_capacity * sizeof(int));
        if (!new_data) {
            return;
        }
        h->data = new_data;
        h->capacity = new_capacity;
    }
    h->data[h->size] = value;
    size_t index = h->size;
    h->size++;

    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (h->data[index] >= h->data[parent]) {
            break;
        }
        
        int temp = h->data[index];
        h->data[index] = h->data[parent];
        h->data[parent] = temp;
        index = parent;
    }
}

int pop_heap(Heap *h) {
    if (!h || is_empty_heap(h)) {
        return -1;
    }
    
    int root = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        
        size_t index = 0;
        while (1) {
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;
            size_t smallest = index;
            
            if (left < h->size && h->data[left] < h->data[smallest]) {
                smallest = left;
            }
            if (right < h->size && h->data[right] < h->data[smallest]) {
                smallest = right;
            }
            if (smallest == index) {
                break;
            }

            int temp = h->data[index];
            h->data[index] = h->data[smallest];
            h->data[smallest] = temp;
            index = smallest;
        }
    }
    return root;
}