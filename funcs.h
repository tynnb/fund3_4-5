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

typedef enum {
    REGULAR,
    URGENT
} LetterType;

typedef enum {
    IN_TRANSIT,
    DELIVERED,
    UNDELIVERED
} LetterState;

typedef enum {
    SUCCESS,
    ERROR_INVALID_ID,
    ERROR_DUPLICATE_OFFICE,
    ERROR_MEMORY_ALLOCATION,
    ERROR_OFFICE_NOT_FOUND,
    ERROR_INVALID_PARAMETER,
    ERROR_OFFICE_FULL,
    ERROR_FILE_OPERATION
} StatusCode;

typedef struct {
    int id;
    LetterType type;
    LetterState state;
    int priority;
    int from_office;
    int to_office;
    int current_office;
    char tech_data[TECH_DATA_SIZE];
} Letter;

typedef struct PostOffice {
    int id;
    int capacity;
    int current_letters;
    int num_connections;
    int *connections;
    Heap letter_heap;
    struct PostOffice *next;
} PostOffice;

typedef struct {
    PostOffice *offices;
    Letter *letters;
    size_t letters_size;
    size_t letters_capacity;
    int next_letter_id;
    FILE *log_file;
} MailSystem;

Heap create_heap(size_t initial_capacity);
void delete_heap(Heap *h);
int is_empty_heap(const Heap *h);
size_t size_heap(const Heap *h);
int peek_heap(const Heap *h);
void push_heap(Heap *h, int value);
int pop_heap(Heap *h);
int remove_letter_from_heap(Heap *heap, int letter_id);

PostOffice* find_office(const MailSystem *system, int office_id);
StatusCode add_office(MailSystem *system, int id, int capacity, int* connections, int num_conn);
StatusCode remove_office(MailSystem *system, int office_id);

Letter* find_letter(MailSystem *system, int letter_id);
StatusCode add_letter(MailSystem *system, LetterType type, int priority, int from_office, int to_office, const char* tech_data);
StatusCode transfer_letter_to_office(MailSystem *system, int letter_id, int from_office_id, int to_office_id);
void process_letters_transfer(MailSystem *system);
void transfer_priority_letters(MailSystem *system);

#endif