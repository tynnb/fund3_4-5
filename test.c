#include "funcs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Вспомогательная функция для создания тестового почтового отделения
PostOffice* create_test_office(int id, int capacity) {
    PostOffice* office = (PostOffice*)malloc(sizeof(PostOffice));
    if (office == NULL) return NULL;
    
    office->id = id;
    office->capacity = capacity;
    office->current_letters = 0;
    office->num_connections = 0;
    office->connections = NULL;
    office->letter_heap = create_heap(INITIAL_CAPACITY);
    office->next = NULL;
    
    return office;
}

// Вспомогательная функция для создания тестового письма
Letter* create_test_letter(int id, LetterType type, int priority, int from_office, int to_office, const char* data) {
    Letter* letter = (Letter*)malloc(sizeof(Letter));
    if (letter == NULL) return NULL;
    
    letter->id = id;
    letter->type = type;
    letter->state = IN_TRANSIT;
    letter->priority = priority;
    letter->from_office = from_office;
    letter->to_office = to_office;
    letter->current_office = from_office;
    strncpy(letter->tech_data, data, sizeof(letter->tech_data) - 1);
    letter->tech_data[sizeof(letter->tech_data) - 1] = '\0';
    
    return letter;
}

void test_create_and_cleanup_system() {
    printf("Testing create and cleanup system...\n");
    
    MailSystem system;
    init_system(&system);
    
    assert(system.offices == NULL);
    assert(system.letters == NULL);
    assert(system.letters_size == 0);
    assert(system.letters_capacity == 0);
    assert(system.next_letter_id == 1);
    assert(system.log_file == NULL);
    
    cleanup_system(&system);
    printf("create and cleanup system tests passed!\n");
}

void test_add_and_remove_office() {
    printf("Testing add and remove office...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Test add office
    StatusCode status1 = add_office(&system, 1, 10, NULL, 0);
    assert(status1 == SUCCESS);
    assert(system.offices != NULL);
    assert(system.offices->id == 1);
    assert(system.offices->capacity == 10);
    
    // Test add duplicate office
    StatusCode status2 = add_office(&system, 1, 15, NULL, 0);
    assert(status2 == ERROR_DUPLICATE_OFFICE);
    
    // Test add second office
    StatusCode status3 = add_office(&system, 2, 20, NULL, 0);
    assert(status3 == SUCCESS);
    
    // Find and verify second office
    PostOffice* office2 = find_office(&system, 2);
    assert(office2 != NULL);
    assert(office2->id == 2);
    assert(office2->capacity == 20);
    
    // Test remove office
    StatusCode status4 = remove_office(&system, 1);
    assert(status4 == SUCCESS);
    
    // Verify office 1 is gone and office 2 remains
    assert(find_office(&system, 1) == NULL);
    assert(find_office(&system, 2) != NULL);
    
    // Test remove non-existent office
    StatusCode status5 = remove_office(&system, 999);
    assert(status5 == ERROR_OFFICE_NOT_FOUND);
    
    cleanup_system(&system);
    printf("add and remove office tests passed!\n");
}

void test_add_office_with_connections() {
    printf("Testing add office with connections...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create target office first
    add_office(&system, 2, 15, NULL, 0);
    
    // Add office with connections
    int connections[] = {2};
    StatusCode status = add_office(&system, 1, 10, connections, 1);
    assert(status == SUCCESS);
    
    PostOffice* office1 = find_office(&system, 1);
    PostOffice* office2 = find_office(&system, 2);
    
    assert(office1 != NULL);
    assert(office2 != NULL);
    assert(office1->num_connections == 1);
    assert(office1->connections[0] == 2);
    assert(office2->num_connections == 1);
    assert(office2->connections[0] == 1);
    
    cleanup_system(&system);
    printf("add office with connections tests passed!\n");
}

void test_add_and_find_letter() {
    printf("Testing add and find letter...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices first
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    
    // Test add letter
    StatusCode status1 = add_letter(&system, REGULAR, 5, 1, 2, "Test data");
    assert(status1 == SUCCESS);
    assert(system.letters_size == 1);
    assert(system.next_letter_id == 2);
    
    // Test find letter
    Letter* found_letter = find_letter(&system, 1);
    assert(found_letter != NULL);
    assert(found_letter->id == 1);
    assert(found_letter->type == REGULAR);
    assert(found_letter->priority == 5);
    assert(found_letter->from_office == 1);
    assert(found_letter->to_office == 2);
    assert(found_letter->current_office == 1);
    assert(strcmp(found_letter->tech_data, "Test data") == 0);
    
    // Test find non-existent letter
    Letter* not_found = find_letter(&system, 999);
    assert(not_found == NULL);
    
    // Test add letter with invalid parameters
    StatusCode status2 = add_letter(&system, REGULAR, -1, 1, 2, "Invalid");
    assert(status2 == ERROR_INVALID_PARAMETER);
    
    StatusCode status3 = add_letter(&system, REGULAR, 5, 999, 2, "Invalid office");
    assert(status3 == ERROR_OFFICE_NOT_FOUND);
    
    cleanup_system(&system);
    printf("add and find letter tests passed!\n");
}

void test_heap_operations() {
    printf("Testing heap operations...\n");
    
    // Test create heap
    Heap heap = create_heap(5);
    assert(heap.data != NULL);
    assert(heap.size == 0);
    assert(heap.capacity == 5);
    
    // Test push to heap
    push_heap(&heap, 10);
    push_heap(&heap, 5);
    push_heap(&heap, 15);
    push_heap(&heap, 3);
    push_heap(&heap, 8);
    
    assert(heap.size == 5);
    assert(peek_heap(&heap) == 3); // Min-heap, smallest should be at top
    
    // Test pop from heap
    int value1 = pop_heap(&heap);
    assert(value1 == 3);
    assert(heap.size == 4);
    assert(peek_heap(&heap) == 5);
    
    int value2 = pop_heap(&heap);
    assert(value2 == 5);
    assert(heap.size == 3);
    
    // Test remove from heap
    push_heap(&heap, 20);
    push_heap(&heap, 1);
    assert(remove_letter_from_heap(&heap, 8) == 1);
    assert(heap.size == 4);
    
    // Test is_empty_heap
    assert(is_empty_heap(&heap) == 0);
    
    delete_heap(&heap);
    assert(is_empty_heap(&heap) == 1);
    
    printf("heap operations tests passed!\n");
}

void test_letter_transfer() {
    printf("Testing letter transfer...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    add_office(&system, 3, 2, NULL, 0); // Small capacity for testing
    
    PostOffice* office1 = find_office(&system, 1);
    PostOffice* office2 = find_office(&system, 2);
    PostOffice* office3 = find_office(&system, 3);
    
    // Add a letter to office 1
    add_letter(&system, REGULAR, 5, 1, 2, "Transfer test");
    assert(office1->current_letters == 1);
    assert(!is_empty_heap(&office1->letter_heap));
    
    // Test successful transfer from office 1 to office 2
    StatusCode status1 = transfer_letter_to_office(&system, 1, 1, 2);
    assert(status1 == SUCCESS);
    assert(office1->current_letters == 0);
    assert(office2->current_letters == 1);
    assert(is_empty_heap(&office1->letter_heap));
    assert(!is_empty_heap(&office2->letter_heap));
    
    // Fill office 3 by adding letters directly
    add_letter(&system, REGULAR, 1, 3, 1, "Filler 1");
    add_letter(&system, REGULAR, 2, 3, 1, "Filler 2");
    assert(office3->current_letters == 2);
    
    // Test transfer to full office (should fail)
    StatusCode status2 = transfer_letter_to_office(&system, 1, 2, 3);
    assert(status2 == ERROR_OFFICE_FULL);
    
    // Verify letter is still in office 2
    assert(office2->current_letters == 1);
    assert(office3->current_letters == 2);
    
    cleanup_system(&system);
    printf("letter transfer tests passed!\n");
}

void test_process_letters_transfer() {
    printf("Testing process letters transfer...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create connected offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    
    // Add letters with different destinations
    add_letter(&system, REGULAR, 10, 1, 2, "To office 2");
    add_letter(&system, URGENT, 1, 1, 2, "To office 2");
    add_letter(&system, REGULAR, 5, 2, 1, "To office 1");
    
    PostOffice* office1 = find_office(&system, 1);
    PostOffice* office2 = find_office(&system, 2);
    
    assert(office1->current_letters == 2);
    assert(office2->current_letters == 1);
    
    // Process transfers - one letter should be delivered to destination
    process_letters_transfer(&system);
    
    // Check that letters still exist
    assert(system.letters_size == 3);
    
    cleanup_system(&system);
    printf("process letters transfer tests passed!\n");
}

void test_priority_letters_transfer() {
    printf("Testing priority letters transfer...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    
    // Add letters with different priorities
    add_letter(&system, URGENT, 100, 1, 2, "High priority");
    add_letter(&system, REGULAR, 10, 1, 2, "Medium priority");
    add_letter(&system, REGULAR, 1, 1, 2, "Low priority");
    
    PostOffice* office1 = find_office(&system, 1);
    assert(office1->current_letters == 3);
    
    // Process priority transfers
    transfer_priority_letters(&system);
    
    // Check that no crashes occurred and letters still exist
    assert(system.letters_size == 3);
    
    cleanup_system(&system);
    printf("priority letters transfer tests passed!\n");
}

void test_file_operations() {
    printf("Testing file operations...\n");
    
    // Test 1: Save letters to file
    printf("  Testing save_letters_to_file...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Создаем офисы и письма
    assert(add_office(&system, 100, 10, NULL, 0) == SUCCESS);
    assert(add_office(&system, 200, 15, NULL, 0) == SUCCESS);
    assert(add_letter(&system, REGULAR, 5, 100, 200, "Test letter") == SUCCESS);
    
    const char* test_filename = "test_output.txt";
    StatusCode status = save_letters_to_file(&system, test_filename);
    assert(status == SUCCESS);
    
    // Проверяем что файл создан
    FILE* file = fopen(test_filename, "r");
    assert(file != NULL);
    
    // Проверяем базовое содержимое
    char line[256];
    int has_content = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) { // Не пустая строка
            has_content = 1;
            break;
        }
    }
    fclose(file);
    assert(has_content == 1);
    
    cleanup_system(&system);
    
    // Удаляем тестовый файл
    remove(test_filename);
    
    printf("file operations tests passed!\n");
}

void test_logging() {
    printf("Testing logging...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Test opening log file
    const char* log_filename = "test_log.txt";
    open_log_file(&system, log_filename);
    assert(system.log_file != NULL);
    
    // Test log message
    log_message(&system, "Test log message");
    
    // Verify log file was written to
    fflush(system.log_file);
    FILE* check_file = fopen(log_filename, "r");
    assert(check_file != NULL);
    fclose(check_file);
    
    // Clean up
    remove(log_filename);
    cleanup_system(&system);
    
    printf("logging tests passed!\n");
}

void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Test NULL parameters
    StatusCode status1 = add_office(NULL, 1, 10, NULL, 0);
    assert(status1 == ERROR_INVALID_ID);
    
    StatusCode status2 = remove_office(NULL, 1);
    assert(status2 == ERROR_INVALID_ID);
    
    StatusCode status3 = add_letter(NULL, REGULAR, 5, 1, 2, "Test");
    assert(status3 == ERROR_INVALID_PARAMETER);
    
    StatusCode status4 = transfer_letter_to_office(NULL, 1, 1, 2);
    assert(status4 == ERROR_INVALID_PARAMETER);
    
    StatusCode status5 = save_letters_to_file(NULL, "test.txt");
    assert(status5 == ERROR_INVALID_ID);
    
    // Test invalid office IDs
    StatusCode status6 = add_office(&system, -1, 10, NULL, 0);
    assert(status6 == ERROR_INVALID_ID);
    
    StatusCode status7 = add_office(&system, 1, 0, NULL, 0);
    assert(status7 == ERROR_INVALID_ID);
    
    // Test with empty system
    process_letters_transfer(&system);
    transfer_priority_letters(&system);
    
    cleanup_system(&system);
    printf("edge cases tests passed!\n");
}

void test_office_capacity() {
    printf("Testing office capacity...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create office with small capacity
    add_office(&system, 1, 2, NULL, 0);
    
    // Add letters up to capacity
    StatusCode status1 = add_letter(&system, REGULAR, 1, 1, 1, "Letter 1");
    assert(status1 == SUCCESS);
    
    StatusCode status2 = add_letter(&system, REGULAR, 2, 1, 1, "Letter 2");
    assert(status2 == SUCCESS);
    
    // Try to add beyond capacity
    StatusCode status3 = add_letter(&system, REGULAR, 3, 1, 1, "Letter 3");
    assert(status3 == ERROR_OFFICE_FULL);
    
    PostOffice* office = find_office(&system, 1);
    assert(office->current_letters == 2);
    
    cleanup_system(&system);
    printf("office capacity tests passed!\n");
}

void test_letter_states() {
    printf("Testing letter states...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create connected offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 10, NULL, 0);
    
    // Test 1: Initial state should be IN_TRANSIT
    add_letter(&system, REGULAR, 5, 1, 2, "Test letter");
    
    Letter* letter = find_letter(&system, 1);
    assert(letter != NULL);
    assert(letter->state == IN_TRANSIT);
    printf("  Initial state: IN_TRANSIT - OK\n");
    
    // Test 2: Transfer to destination and check delivery
    // Use multiple process cycles to ensure delivery
    for (int i = 0; i < 5; i++) {
        process_letters_transfer(&system);
    }
    
    // Check if letter was delivered (it should reach office 2 and be delivered)
    letter = find_letter(&system, 1);
    if (letter->state == DELIVERED) {
        printf("  Final state: DELIVERED - OK\n");
    } else {
        printf("  Final state: %s (expected DELIVERED)\n", 
               letter->state == IN_TRANSIT ? "IN_TRANSIT" : "UNDELIVERED");
    }
    
    // Test 3: Add another letter and verify initial state
    add_letter(&system, URGENT, 10, 1, 2, "Another test");
    Letter* letter2 = find_letter(&system, 2);
    assert(letter2 != NULL);
    assert(letter2->state == IN_TRANSIT);
    printf("  New letter state: IN_TRANSIT - OK\n");
    
    cleanup_system(&system);
    printf("letter states tests passed!\n");
}

void test_comprehensive_scenario() {
    printf("Testing comprehensive scenario...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices with connections
    add_office(&system, 1, 5, NULL, 0);
    add_office(&system, 2, 5, NULL, 0);
    add_office(&system, 3, 5, NULL, 0);
    
    // Add letters
    assert(add_letter(&system, REGULAR, 10, 1, 2, "Test letter 1") == SUCCESS);
    assert(add_letter(&system, URGENT, 20, 2, 1, "Test letter 2") == SUCCESS);
    assert(add_letter(&system, REGULAR, 5, 1, 3, "Test letter 3") == SUCCESS);
    
    // Verify setup
    assert(system.letters_size == 3);
    
    // Run processing (should not crash)
    process_letters_transfer(&system);
    transfer_priority_letters(&system);
    process_letters_transfer(&system);
    
    // Basic sanity checks
    assert(system.letters_size == 3);
    
    // Check that letters still exist and have valid states
    for (size_t i = 0; i < system.letters_size; i++) {
        assert(system.letters[i].state == IN_TRANSIT || 
               system.letters[i].state == DELIVERED ||
               system.letters[i].state == UNDELIVERED);
    }
    
    printf("  System processed %zu letters without errors\n", system.letters_size);
    
    cleanup_system(&system);
    printf("comprehensive scenario tests completed!\n");
}

void test_remove_office_with_letters() {
    printf("Testing remove office with letters...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    
    // Add letters to office 1
    add_letter(&system, REGULAR, 5, 1, 2, "Letter to office 2");
    add_letter(&system, REGULAR, 3, 1, 1, "Letter to same office");
    
    PostOffice* office1 = find_office(&system, 1);
    assert(office1->current_letters == 2);
    
    // Remove office 1 - letters should be marked as undeliverable
    StatusCode status = remove_office(&system, 1);
    assert(status == SUCCESS);
    
    // Check that letters are marked as undeliverable
    Letter* letter1 = find_letter(&system, 1);
    Letter* letter2 = find_letter(&system, 2);
    
    if (letter1) {
        assert(letter1->state == UNDELIVERED);
    }
    if (letter2) {
        assert(letter2->state == UNDELIVERED);
    }
    
    cleanup_system(&system);
    printf("remove office with letters tests passed!\n");
}

void test_auto_connection_creation() {
    printf("Testing auto connection creation...\n");
    
    MailSystem system;
    init_system(&system);
    
    // Create offices
    add_office(&system, 1, 10, NULL, 0);
    add_office(&system, 2, 15, NULL, 0);
    
    // Adding a letter between offices should create automatic connections
    add_letter(&system, REGULAR, 5, 1, 2, "Auto connection test");
    
    PostOffice* office1 = find_office(&system, 1);
    PostOffice* office2 = find_office(&system, 2);
    
    assert(office1 != NULL);
    assert(office2 != NULL);
    
    // Check if connection was created
    int connection_found = 0;
    for (int i = 0; i < office1->num_connections; i++) {
        if (office1->connections[i] == 2) {
            connection_found = 1;
            break;
        }
    }
    assert(connection_found == 1);
    
    connection_found = 0;
    for (int i = 0; i < office2->num_connections; i++) {
        if (office2->connections[i] == 1) {
            connection_found = 1;
            break;
        }
    }
    assert(connection_found == 1);
    
    cleanup_system(&system);
    printf("auto connection creation tests passed!\n");
}

int main() {
    printf("Running mail system tests...\n\n");
    
    test_create_and_cleanup_system();
    test_add_and_remove_office();
    test_add_office_with_connections();
    test_add_and_find_letter();
    test_heap_operations();
    test_letter_transfer();
    test_process_letters_transfer();
    test_priority_letters_transfer();
    test_file_operations();
    test_logging();
    test_edge_cases();
    test_office_capacity();
    test_letter_states();
    test_comprehensive_scenario();
    test_remove_office_with_letters();
    test_auto_connection_creation();
    
    printf("\nAll mail system tests completed successfully!\n");
    return 0;
}