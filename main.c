#include "funcs.h"

static void print_menu(int auto_transfer_enabled) {
    printf("1. Add a post office\n");
    printf("2. Remove post office\n");
    printf("3. Add a letter\n");
    printf("4. List of all letters\n");
    printf("5. Show office connections\n");
    printf("6. System status\n");
    printf("7. %s delivery\n", auto_transfer_enabled ? "Stop" : "Start");
    printf("8. Exit\n");
    printf("Select an option: ");
}

int main(int argc, char *argv[]) {    
    const char *log_file = "system_log.txt";
    int auto_transfer_enabled = 0;
    
    if (argc > 1) {
        log_file = argv[1];
    }
    
    MailSystem system;
    init_system(&system);
    open_log_file(&system, log_file);
    
    int main_running = 1;
    while (main_running) {
        if (auto_transfer_enabled) {
            transfer_priority_letters(&system);
            msleep(200);
        }
        
        print_menu(auto_transfer_enabled);
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        
        switch (choice) {
            case 1: {
                int id, capacity;
                printf("Enter office ID: ");
                scanf("%d", &id);
                printf("Enter capacity: ");
                scanf("%d", &capacity);
                
                StatusCode status = add_office(&system, id, capacity, NULL, 0);
                if (status != SUCCESS) {
                    printf("Error adding office: %d\n", status);
                } else {
                    printf("The office was added successfully.\n");
                }
                break;
            }
            
            case 2: {
                int id;
                printf("Enter the office ID to delete: ");
                scanf("%d", &id);
                
                StatusCode status = remove_office(&system, id);
                if (status != SUCCESS) {
                    printf("Error deleting office: %d\n", status);
                } else {
                    printf("The branch was removed successfully.\n");
                }
                break;
            }
            
            case 3: {
                int type, priority, from, to;
                char data[100];
                
                printf("Enter the letter type (0-Regular, 1-Urgent): ");
                scanf("%d", &type);
                printf("Enter priority: ");
                scanf("%d", &priority);
                printf("Enter the sender's office ID: ");
                scanf("%d", &from);
                printf("Enter the recipient's office ID: ");
                scanf("%d", &to);
                printf("Enter technical data: ");
                scanf(" %99[^\n]", data);
                
                StatusCode status = add_letter(&system, (LetterType)type, priority, from, to, data);
                if (status != SUCCESS) {
                    printf("Error adding letter: %d\n", status);
                } else {
                    printf("The letter was added successfully.\n");
                }
                break;
            }
            
            case 4: {
                char filename[100];
                printf("Enter the output file name: ");
                scanf(" %99[^\n]", filename);
                
                StatusCode status = save_letters_to_file(&system, filename);
                if (status != SUCCESS) {
                    printf("Error saving email list: %d\n", status);
                } else {
                    printf("The list of letters has been saved.\n");
                }
                break;
            }
            
            case 5: {
                int office_id;
                printf("Enter office ID: ");
                scanf("%d", &office_id);
                print_office_connections(&system, office_id);
                break;
            }
            
            case 6: {
                print_system_status(&system, auto_transfer_enabled);
                break;
            }
            
            case 7: {
                auto_transfer_enabled = !auto_transfer_enabled;
                if (auto_transfer_enabled) {
                    printf("delivery on\n");
                } else {
                    printf("delivery off\n");
                }
                break;
            }
            
            case 8:
                main_running = 0;
                printf("Exit\n");
                break;
                
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
        
        if (!auto_transfer_enabled) {
            msleep(100);
        }
    }
    cleanup_system(&system);
    return 0;
}