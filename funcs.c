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

int remove_letter_from_heap(Heap *heap, int letter_id) {
    if (!heap) {
        return 0;
    }
    
    Heap temp_heap = create_heap(heap->capacity);
    int found = 0;
    
    while (!is_empty_heap(heap)) {
        int current_id = pop_heap(heap);
        if (current_id == letter_id) {
            found = 1;
        } else {
            push_heap(&temp_heap, current_id);
        }
    }
    
    while (!is_empty_heap(&temp_heap)) {
        push_heap(heap, pop_heap(&temp_heap));
    }
    delete_heap(&temp_heap);
    return found;
}

PostOffice* find_office(const MailSystem *system, int office_id) {
    if (!system) {
        return NULL;
    }

    PostOffice *current = system->offices;
    while (current) {
        if (current->id == office_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

StatusCode add_office(MailSystem *system, int id, int capacity, int* connections, int num_conn) {
    if (!system || id < 0 || capacity <= 0) {
        return ERROR_INVALID_ID;
    }
    if (find_office(system, id)) {
        return ERROR_DUPLICATE_OFFICE;
    }

    PostOffice *new_office = (PostOffice*)malloc(sizeof(PostOffice));
    if (!new_office) {
        return ERROR_MEMORY_ALLOCATION;
    }
    
    new_office->id = id;
    new_office->capacity = capacity;
    new_office->current_letters = 0;
    new_office->num_connections = 0;
    new_office->letter_heap = create_heap(INITIAL_CAPACITY);
    new_office->next = system->offices;
    system->offices = new_office;
    
    if (num_conn > 0) {
        new_office->connections = (int*)malloc(num_conn * sizeof(int));
        if (!new_office->connections) {
            delete_heap(&new_office->letter_heap);
            free(new_office);
            return ERROR_MEMORY_ALLOCATION;
        }
        
        for (int i = 0; i < num_conn; i++) {
            new_office->connections[i] = connections[i];
            new_office->num_connections++;
            
            PostOffice *target_office = find_office(system, connections[i]);
            if (target_office && target_office->num_connections < MAX_CONNECTIONS) {
                int connection_exists = 0;
                for (int j = 0; j < target_office->num_connections; j++) {
                    if (target_office->connections[j] == id) {
                        connection_exists = 1;
                        break;
                    }
                }
                
                if (!connection_exists) {
                    if (!target_office->connections) {
                        target_office->connections = (int*)malloc(MAX_CONNECTIONS * sizeof(int));
                    }
                    target_office->connections[target_office->num_connections++] = id;
                }
            }
        }
    } else {
        new_office->connections = NULL;
    }
    
    char log_msg[256];
    sprintf(log_msg, "Added office %d with capacity %d", id, capacity);
    log_message(system, log_msg);
    return SUCCESS;
}

StatusCode remove_office(MailSystem *system, int office_id) {
    if (!system) {
        return ERROR_INVALID_ID;
    }
    
    PostOffice **prev = &system->offices;
    PostOffice *current = system->offices;
    
    while (current) {
        if (current->id == office_id) {
            while (!is_empty_heap(&current->letter_heap)) {
                int letter_id = pop_heap(&current->letter_heap);
                Letter *letter = find_letter(system, letter_id);
                if (letter) {
                    if (letter->from_office == office_id || letter->to_office == office_id) {
                        letter->state = UNDELIVERED;
                        char log_msg[256];
                        sprintf(log_msg, "Letter %d marked as undeliverable (office %d removed)", letter_id, office_id);
                        log_message(system, log_msg);
                    } else {
                        int transferred = 0;
                        for (int i = 0; i < current->num_connections && !transferred; i++) {
                            int target_id = current->connections[i];
                            if (transfer_letter_to_office(system, letter_id, office_id, target_id) == SUCCESS) {
                                transferred = 1;
                            }
                        }
                        if (!transferred) {
                            letter->state = UNDELIVERED;
                            char log_msg[256];
                            sprintf(log_msg, "Letter %d marked as undeliverable (no route after office removal)", letter_id);
                            log_message(system, log_msg);
                        }
                    }
                }
            }
            PostOffice *other_office = system->offices;
            while (other_office) {
                if (other_office->id != office_id) {
                    for (int i = 0; i < other_office->num_connections; i++) {
                        if (other_office->connections[i] == office_id) {
                            for (int j = i; j < other_office->num_connections - 1; j++) {
                                other_office->connections[j] = other_office->connections[j + 1];
                            }
                            other_office->num_connections--;
                            
                            if (other_office->num_connections > 0) {
                                int *new_connections = (int*)realloc(other_office->connections, other_office->num_connections * sizeof(int));
                                if (new_connections) {
                                    other_office->connections = new_connections;
                                }
                            } else {
                                free(other_office->connections);
                                other_office->connections = NULL;
                            }
                            break;
                        }
                    }
                }
                other_office = other_office->next;
            }

            *prev = current->next;
            delete_heap(&current->letter_heap);
            free(current->connections);
            free(current);
            
            char log_msg[256];
            sprintf(log_msg, "Removed office %d", office_id);
            log_message(system, log_msg);
            return SUCCESS;
        }
        prev = &current->next;
        current = current->next;
    }
    return ERROR_OFFICE_NOT_FOUND;
}

Letter* find_letter(MailSystem *system, int letter_id) {
    if (!system) {
        return NULL;
    }

    for (size_t i = 0; i < system->letters_size; i++) {
        if (system->letters[i].id == letter_id) {
            return &system->letters[i];
        }
    }
    return NULL;
}

StatusCode add_letter(MailSystem *system, LetterType type, int priority, int from_office, int to_office, const char* tech_data) {
    if (!system || priority < 0 || !tech_data) {
        return ERROR_INVALID_PARAMETER;
    }
    
    PostOffice *from_office_ptr = find_office(system, from_office);
    PostOffice *to_office_ptr = find_office(system, to_office);
    if (!from_office_ptr || !to_office_ptr) {
        return ERROR_OFFICE_NOT_FOUND;
    }

    int connection_exists = 0;
    for (int i = 0; i < from_office_ptr->num_connections; i++) {
        if (from_office_ptr->connections[i] == to_office) {
            connection_exists = 1;
            break;
        }
    }
    
    if (!connection_exists) {
        if (from_office_ptr->num_connections < MAX_CONNECTIONS) {
            if (!from_office_ptr->connections) {
                from_office_ptr->connections = (int*)malloc(MAX_CONNECTIONS * sizeof(int));
            }
            from_office_ptr->connections[from_office_ptr->num_connections++] = to_office;
            
            char log_msg[256];
            sprintf(log_msg, "Auto-created connection: office %d -> office %d", from_office, to_office);
            log_message(system, log_msg);
        }
        
        if (to_office_ptr->num_connections < MAX_CONNECTIONS) {
            int reverse_connection_exists = 0;
            for (int i = 0; i < to_office_ptr->num_connections; i++) {
                if (to_office_ptr->connections[i] == from_office) {
                    reverse_connection_exists = 1;
                    break;
                }
            }
            
            if (!reverse_connection_exists) {
                if (!to_office_ptr->connections) {
                    to_office_ptr->connections = (int*)malloc(MAX_CONNECTIONS * sizeof(int));
                }
                to_office_ptr->connections[to_office_ptr->num_connections++] = from_office;
                
                char log_msg[256];
                sprintf(log_msg, "Auto-created connection: office %d -> office %d", to_office, from_office);
                log_message(system, log_msg);
            }
        }
    }

    if (system->letters_size >= system->letters_capacity) {
        size_t new_capacity = system->letters_capacity == 0 ? 10 : system->letters_capacity * 2;
        Letter *new_letters = (Letter*)realloc(system->letters, new_capacity * sizeof(Letter));
        if (!new_letters) {
            return ERROR_MEMORY_ALLOCATION;
        }
        system->letters = new_letters;
        system->letters_capacity = new_capacity;
    }
    
    Letter *new_letter = &system->letters[system->letters_size];
    new_letter->id = system->next_letter_id++;
    new_letter->type = type;
    new_letter->state = IN_TRANSIT;
    new_letter->priority = priority;
    new_letter->from_office = from_office;
    new_letter->to_office = to_office;
    new_letter->current_office = from_office;
    strncpy(new_letter->tech_data, tech_data, sizeof(new_letter->tech_data) - 1);
    new_letter->tech_data[sizeof(new_letter->tech_data) - 1] = '\0';
    
    if (from_office_ptr->current_letters >= from_office_ptr->capacity) {
        return ERROR_OFFICE_FULL;
    }
    
    push_heap(&from_office_ptr->letter_heap, new_letter->id);
    from_office_ptr->current_letters++;
    system->letters_size++;
    
    char log_msg[256];
    sprintf(log_msg, "Added letter %d from office %d to office %d", new_letter->id, from_office, to_office);
    log_message(system, log_msg);
    return SUCCESS;
}

StatusCode transfer_letter_to_office(MailSystem *system, int letter_id, int from_office_id, int to_office_id) {
    if (!system) {
        return ERROR_INVALID_PARAMETER;
    }
    
    PostOffice *from_office = find_office(system, from_office_id);
    PostOffice *target_office = find_office(system, to_office_id);
    if (!target_office) {
        return ERROR_OFFICE_NOT_FOUND;
    }
    
    if (target_office->current_letters >= target_office->capacity) {
        return ERROR_OFFICE_FULL;
    }
    
    if (from_office && remove_letter_from_heap(&from_office->letter_heap, letter_id)) {
        from_office->current_letters--;
    }
    
    push_heap(&target_office->letter_heap, letter_id);
    target_office->current_letters++;
    
    Letter *letter = find_letter(system, letter_id);
    if (letter) {
        letter->current_office = to_office_id;
    }
    
    char log_msg[256];
    sprintf(log_msg, "Letter %d transferred from office %d to office %d", letter_id, from_office_id, to_office_id);
    log_message(system, log_msg);
    return SUCCESS;
}

void process_letters_transfer(MailSystem *system) {
    if (!system) {
        return;
    }
    
    PostOffice *office = system->offices;
    while (office) {
        Heap temp_heap = create_heap(office->letter_heap.capacity);
        int *letter_ids = NULL;
        int *letter_priorities = NULL;
        size_t count = 0;
        
        while (!is_empty_heap(&office->letter_heap)) {
            int letter_id = pop_heap(&office->letter_heap);
            Letter *letter = find_letter(system, letter_id);
            
            if (letter) {
                int *temp_ids = realloc(letter_ids, (count + 1) * sizeof(int));
                if (!temp_ids) {
                    free(letter_priorities);
                    free(letter_ids);
                    return;
                }
                letter_ids = temp_ids;
                int *temp_priorities = realloc(letter_priorities, (count + 1) * sizeof(int));
                if (!temp_priorities) {
                    free(letter_ids);
                    free(letter_priorities);
                    return;
                }
                letter_priorities = temp_priorities;
                letter_ids[count] = letter_id;
                letter_priorities[count] = letter->priority;
                count++;
            }
            push_heap(&temp_heap, letter_id);
        }

        while (!is_empty_heap(&temp_heap)) {
            push_heap(&office->letter_heap, pop_heap(&temp_heap));
        }
        delete_heap(&temp_heap);
        sort_by_priority(letter_ids, letter_priorities, NULL, count);
        for (size_t i = 0; i < count; i++) {
            int letter_id = letter_ids[i];
            Letter *letter = find_letter(system, letter_id);
            
            if (!letter || letter->state != IN_TRANSIT) {
                continue;
            }
            if (!remove_letter_from_heap(&office->letter_heap, letter_id)) {
                continue;
            }
            office->current_letters--;
            
            if (letter->to_office == office->id) {
                letter->state = DELIVERED;
                push_heap(&office->letter_heap, letter_id);
                office->current_letters++;
                
                char log_msg[256];
                sprintf(log_msg, "Letter %d delivered to office %d", letter_id, office->id);
                log_message(system, log_msg);
            } else {
                int transferred = 0;
                
                PostOffice *target_office = find_office(system, letter->to_office);
                if (target_office && target_office->current_letters < target_office->capacity) {
                    if (transfer_letter_to_office(system, letter_id, office->id, letter->to_office) == SUCCESS) {
                        transferred = 1;
                    }
                }

                if (!transferred && office->num_connections > 0) {
                    for (int j = 0; j < office->num_connections && !transferred; j++) {
                        int next_office_id = office->connections[j];
                        PostOffice *next_office = find_office(system, next_office_id);
                        
                        if (next_office && next_office->current_letters < next_office->capacity) {
                            if (transfer_letter_to_office(system, letter_id, office->id, next_office_id) == SUCCESS) {
                                transferred = 1;
                            }
                        }
                    }
                }
                if (!transferred) {
                    push_heap(&office->letter_heap, letter_id);
                    office->current_letters++;
                }
            }
            break;
        }
        free(letter_ids);
        free(letter_priorities);
        office = office->next;
    }
}

void transfer_priority_letters(MailSystem *system) {
    if (!system) {
        return;
    }

    int total_letters = 0;
    int max_letters = system->letters_size * 2;
    int *all_letter_ids = malloc(max_letters * sizeof(int));
    int *all_letter_priorities = malloc(max_letters * sizeof(int));
    PostOffice **letter_offices = malloc(max_letters * sizeof(PostOffice*));
    
    if (!all_letter_ids || !all_letter_priorities || !letter_offices) {
        free(all_letter_ids);
        free(all_letter_priorities);
        free(letter_offices);
        return;
    }

    PostOffice *office = system->offices;
    while (office) {
        Heap temp_heap = create_heap(office->letter_heap.capacity);

        while (!is_empty_heap(&office->letter_heap)) {
            int letter_id = pop_heap(&office->letter_heap);
            Letter *letter = find_letter(system, letter_id);
            
            if (letter && letter->state == IN_TRANSIT) {
                all_letter_ids[total_letters] = letter_id;
                all_letter_priorities[total_letters] = letter->priority;
                letter_offices[total_letters] = office;
                total_letters++;
            }
            push_heap(&temp_heap, letter_id);
        }

        while (!is_empty_heap(&temp_heap)) {
            push_heap(&office->letter_heap, pop_heap(&temp_heap));
        }
        delete_heap(&temp_heap);
        office = office->next;
    }

    sort_by_priority(all_letter_ids, all_letter_priorities, letter_offices, total_letters);
    int letters_processed = 0;
    int max_to_process = 1;
    
    for (int i = 0; i < total_letters && letters_processed < max_to_process; i++) {
        int letter_id = all_letter_ids[i];
        PostOffice *current_office = letter_offices[i];
        Letter *letter = find_letter(system, letter_id);
        
        if (!letter || letter->state != IN_TRANSIT) {
            continue;
        }
        if (!remove_letter_from_heap(&current_office->letter_heap, letter_id)) {
            continue;
        }

        if (letter->to_office == current_office->id) {
            letter->state = DELIVERED;
            current_office->current_letters--;
            
            char log_msg[256];
            sprintf(log_msg, "Letter %d delivered to office %d (priority: %d)", letter->id, current_office->id, letter->priority);
            log_message(system, log_msg);
            letters_processed++;
            continue;
        }

        PostOffice *best_next_office = NULL;
        int best_score = -1;

        PostOffice *target_office = find_office(system, letter->to_office);
        if (target_office && target_office->current_letters < target_office->capacity) {
            best_next_office = target_office;
            best_score = 1000;
        }

        if (!best_next_office) {
            for (int j = 0; j < current_office->num_connections; j++) {
                int next_office_id = current_office->connections[j];
                PostOffice *next_office = find_office(system, next_office_id);
                
                if (next_office && next_office->current_letters < next_office->capacity) {
                    int score = 0;

                    for (int k = 0; k < next_office->num_connections; k++) {
                        if (next_office->connections[k] == letter->to_office) {
                            score += 100;
                            break;
                        }
                    }

                    int distance = abs(next_office->id - letter->to_office);
                    score += (10 - distance);

                    int free_capacity = next_office->capacity - next_office->current_letters;
                    score += free_capacity;
                    
                    if (score > best_score) {
                        best_score = score;
                        best_next_office = next_office;
                    }
                }
            }
        }

        if (best_next_office) {
            push_heap(&best_next_office->letter_heap, letter_id);
            best_next_office->current_letters++;
            letter->current_office = best_next_office->id;
            
            char log_msg[256];
            sprintf(log_msg, "Letter %d transferred from %d to %d (priority: %d)", letter->id, current_office->id, best_next_office->id, letter->priority);
            log_message(system, log_msg);
            letters_processed++;
        } else {
            push_heap(&current_office->letter_heap, letter_id);
        }
    }
    free(all_letter_ids);
    free(all_letter_priorities);
    free(letter_offices);
}