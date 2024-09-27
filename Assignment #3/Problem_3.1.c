#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>

// To easily set the max number of threads
#define MAX_THREADS 10000

typedef struct {
    int start;
    int end;
    int id;
    int vb; // Verbose Flag
} Thread_I_D;

// Function to check if a number is a narcissistic/ PDI number
int pdi_search(int n) {
    int sum = 0;
    int starter = n;
    int i = 0;

    // Count the number of digits
    while (n) {
        n /= 10;
        i++;
    }

    n = starter;
    while (n) {
        int digit = n % 10;
        int power = 1;

        // Compute digit^i manually
        for (int j = 0; j < i; j++) {
            power *= digit;
        }

        sum += power;
        n /= 10;
    }
    return sum == starter;
}

// Function to search for PDI numbers in the given range of each thread
void* search_pdi(void *arg) {
    Thread_I_D *call = (Thread_I_D*) arg;
    if (call->vb) {
        fprintf(stderr, "pdi-numbers: t%d searching [%d, %d]\n", call->id, call->start, call->end);
    }
    for (int i = call->start; i <= call->end; i++) {
        if (pdi_search(i)) {
            printf("%d\n", i); // Print Number if Found
        }
    }
    if (call->vb) {
        fprintf(stderr, "t%d finished\n", call->id);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int start = 1;
    int end = 10000;
    int threads = 1;
    int vb = 0;
    int opt;

    opterr = 0;

    // Parsing using getopt
    while ((opt = getopt(argc, argv, ":s:e:t:v")) != -1) {
        switch (opt) {
            case 's':
                start = atoi(optarg);
                break;
            case 'e':
                end = atoi(optarg);
                break;
            case 't':
                threads = atoi(optarg);
                if (threads > MAX_THREADS) {
                    fprintf(stderr, "Error: Maximum number of threads is %d\n", MAX_THREADS);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                vb = 1; // Turning on verbose flag
                break;
            case '?':
                fprintf(stderr, "Error: Unknown option -%c\n", optopt);
                exit(EXIT_FAILURE);
            case ':':
                fprintf(stderr, "Error: Option -%c requires an argument\n", optopt);
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "Usage: %s [-s start] [-e end] [-t threads] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Creating an array of threads
    pthread_t thread[MAX_THREADS];
    Thread_I_D call[MAX_THREADS];
    int range = (end - start + 1) / threads;
    int remainder = (end - start + 1) % threads;

    for (int i = 0; i < threads; i++) {
        call[i].start = start + i * range;
        call[i].end = (i == threads - 1) ? end : call[i].start + range - 1;
        if (i == threads - 1) {
            call[i].end = end;  // Assign the remainder to the last thread
        }
        call[i].id = i;
        call[i].vb = vb;
        if (pthread_create(&thread[i], NULL, search_pdi, &call[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < threads; i++) {
        if (pthread_join(thread[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
