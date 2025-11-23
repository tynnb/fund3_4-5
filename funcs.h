#ifndef FUNCS_H
#define FUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 10
#define MAX_CONNECTIONS 100
#define TECH_DATA_SIZE 256

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Heap;

Heap create_heap(size_t initial_capacity);
void delete_heap(Heap *h);
int is_empty_heap(const Heap *h);
size_t size_heap(const Heap *h);
int peek_heap(const Heap *h);
void push_heap(Heap *h, int value);
int pop_heap(Heap *h);

#endif