#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_STUDENTS 2
#define DEFAULT_CAPACITY 4
#define COINS_REQUIRED 5

int total_students = DEFAULT_STUDENTS;
int capacity = DEFAULT_CAPACITY;
int drinks_in_machine = 0;
int coins_collected = 0;
int coins_inserted = 0;
int current_student = -1;

pthread_mutex_t vending_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_insert_coins = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_dispense = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_pickup = PTHREAD_COND_INITIALIZER;
pthread_cond_t need_refill = PTHREAD_COND_INITIALIZER;

void print_status(const char *message) {
    printf("energy: [%d/%d drinks, %d coins, %d inserted] %s\n",drinks_in_machine, capacity, coins_collected, coins_inserted, message);
}

void *machine(void *arg) {
    print_status("machine booting up");
    
    while (1) {
        pthread_mutex_lock(&vending_mutex);
        
        while (coins_inserted < COINS_REQUIRED || drinks_in_machine == 0) {
            if (drinks_in_machine == 0) {
                print_status("machine waiting for refill");
                pthread_cond_signal(&need_refill);
            } else {
                print_status("machine waiting for more coins");
            }
            pthread_cond_wait(&can_dispense, &vending_mutex);
        }
        
        drinks_in_machine--;
        coins_collected += coins_inserted;
        coins_inserted = 0;
        
        print_status("machine dispensing drink");
        pthread_cond_signal(&can_pickup);
        
        pthread_mutex_unlock(&vending_mutex);
    }
    
    return NULL;
}

void *student(void *arg) {
    int id = *(int*)arg;
    char message[100];
    
    snprintf(message, sizeof(message), "student %d established", id);
    print_status(message);
    
    while (1) {
        pthread_mutex_lock(&vending_mutex);
        
        snprintf(message, sizeof(message), "student %d requires an energy drink", id);
        print_status(message);
        
        while (current_student != -1 || drinks_in_machine == 0) {
            if (drinks_in_machine == 0) {
                snprintf(message, sizeof(message), "student %d waiting for machine to be refilled", id);
            } else {
                snprintf(message, sizeof(message), "student %d waiting for the machine", id);
            }
            print_status(message);
            pthread_cond_wait(&can_insert_coins, &vending_mutex);
        }
        
        current_student = id;
        snprintf(message, sizeof(message), "student %d is next to be served", id);
        print_status(message);
        
        while (coins_inserted < COINS_REQUIRED) {
            coins_inserted++;
            snprintf(message, sizeof(message), "student %d inserted another coin", id);
            print_status(message);
        }
        
        snprintf(message, sizeof(message), "student %d waiting for drink to arrive", id);
        print_status(message);
        pthread_cond_signal(&can_dispense);
        pthread_cond_wait(&can_pickup, &vending_mutex);
        
        snprintf(message, sizeof(message), "student %d picked up a drink", id);
        print_status(message);
        
        current_student = -1;
        pthread_cond_signal(&can_insert_coins);
        
        pthread_mutex_unlock(&vending_mutex);
        
        snprintf(message, sizeof(message), "student %d enjoying an energy drink", id);
        print_status(message);
        sleep(1);  // Simulate drinking time
    }
    
    return NULL;
}

void *supplier(void *arg) {
    print_status("supplier established");
    
    while (1) {
        pthread_mutex_lock(&vending_mutex);
        
        while (drinks_in_machine > 0) {
            pthread_cond_wait(&need_refill, &vending_mutex);
        }
        
        print_status("supplier arriving");
        print_status("supplier loading 4 drinks");
        
        for (int i = 0; i < capacity; i++) {
            drinks_in_machine++;
        }
        
        char message[100];
        snprintf(message, sizeof(message), "supplier collected %d coins", coins_collected);
        print_status(message);
        coins_collected = 0;
        
        print_status("supplier leaving");
        pthread_cond_broadcast(&can_insert_coins);
        
        pthread_mutex_unlock(&vending_mutex);
        
        sleep(10);  // Simulate time between refills
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    int option;
    while ((option = getopt(argc, argv, "n:c:")) != -1) {
        switch (option) {
            case 'n':
                total_students = atoi(optarg);
                break;
            case 'c':
                capacity = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Error: Unknown option -%c\n", optopt);
                exit(EXIT_FAILURE);
            case ':':
                fprintf(stderr, "Error: Option -%c requires an argument\n", optopt);
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "Usage: %s [-n total_students] [-c capacity]\\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            total_students = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            capacity = atoi(argv[i + 1]);
            i++;
        }
    }

    pthread_t machine_thread, supplier_thread;
    pthread_t student_threads[total_students];
    int student_ids[total_students];

    pthread_create(&machine_thread, NULL, machine, NULL);
    pthread_create(&supplier_thread, NULL, supplier, NULL);

    for (int i = 0; i < total_students; i++) {
        student_ids[i] = i;
        pthread_create(&student_threads[i], NULL, student, &student_ids[i]);
    }

    pthread_join(machine_thread, NULL);

    return 0;
}