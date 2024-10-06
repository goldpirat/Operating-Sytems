#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>


// This part may be redundant as it is called the 100 prisoner problem
// However the code is much more practical this way.
#define NO_OF_DRAWER 100
#define NO_OF_PRISONERS 100

// Drawers with individual locks
typedef struct d {
    int pid; // Prisoner Draweer ID
    pthread_mutex_t mutex; // Individual Lock
} drawer_t;

// Prisoner Threads and drawers initialization
pthread_t prisoner[NO_OF_PRISONERS];
drawer_t drawer[NO_OF_DRAWER];

// Random number generator variables
static unsigned long random_ = 1;
static unsigned long int succ_r = 0, win_r = 0;
static unsigned long int succ_s = 0, win_s = 0;
static unsigned long int win_rd = 0, succ_rd = 0;
static unsigned long int win_sd = 0, succ_sd = 0;

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER; // global lock

// All Functions To Be Used in Program
int better_rand(void);
void better_rand_two(unsigned seed);
void rand_gen(drawer_t draw[NO_OF_DRAWER]);
void rand_search(int n, drawer_t draw[NO_OF_DRAWER]);
void* random_global(void *arg);
void* random_drawer(void *arg);
void smart_search(int n, drawer_t draw[NO_OF_DRAWER]);
void* smart_global(void *arg);
void* smart_drawer(void *arg);
void run_sim(int n, void* (*proc)(void *));
static double timeit(int n, void* (*proc)(void *));


int main(int argc, char* argv[]) {
    int games = 100; // default number of games
    unsigned seed = time(NULL);
    int opt;

    // Get opt usage
    while ((opt = getopt(argc, argv, "n:s:")) != -1) {
        switch (opt) {
            case 'n': // -n format
                games = atoi(optarg);
                break;
            case 's': // -s format
                seed = atoi(optarg);
                break;
            default:
                printf("Usage: ./prisoner [-s seed] [-n games]\n");
                return EXIT_FAILURE;
        }
    }

    // Set seed for the random number
    better_rand_two(seed);

    // Random_global strategy
    static double rand_time = 0;
    int sppr = 0; // Successful Prisoners Previous Round

    for (int d = 1; d <= games; d++) {
        rand_gen(drawer); // Randomly generate drawer numbers
        for (int i = 0; i < NO_OF_DRAWER; i++)
            pthread_mutex_init(&drawer[i].mutex, NULL);

        rand_time += timeit(NO_OF_PRISONERS, random_global);

        sppr = succ_r - sppr;
        if (sppr == 100) win_r++;
        sppr = succ_r;
    }

    // Random_drawer strategy
    static double rand_drawer_time = 0;
    int sppr_rd = 0; // Successful Prisoners Previous Round for random_drawer

    for (int d = 1; d <= games; d++) {
        rand_gen(drawer);
        for (int i = 0; i < NO_OF_DRAWER; i++)
            pthread_mutex_init(&drawer[i].mutex, NULL);

        rand_drawer_time += timeit(NO_OF_PRISONERS, random_drawer);

        sppr_rd = succ_rd - sppr_rd;
        if (sppr_rd == 100) win_rd++;
        sppr_rd = succ_rd;
    }

    // Smart_global strategy
    static double seq_time = 0;
    int sppr_sg = 0; // Successful Prisoners Previous Round for smart_global

    for (int d = 1; d <= games; d++) {
        rand_gen(drawer);
        for (int i = 0; i < NO_OF_DRAWER; i++)
            pthread_mutex_init(&drawer[i].mutex, NULL);

        seq_time += timeit(NO_OF_PRISONERS, smart_global);

        sppr_sg= succ_s - sppr_sg;
        if (sppr_sg== 100) win_s++;
        sppr_sg= succ_s;
    }

    // Smart_drawer strategy
    static double smart_drawer_time = 0;
    int sppr_sd = 0; // Successful Prisoners Previous Round for smart_drawer

    for (int d = 1; d <= games; d++) {
        rand_gen(drawer);
        for (int i = 0; i < NO_OF_DRAWER; i++)
            pthread_mutex_init(&drawer[i].mutex, NULL);

        smart_drawer_time += timeit(NO_OF_PRISONERS, smart_drawer);

        sppr_sd = succ_sd - sppr_sd;
        if (sppr_sd == 100) win_sd++;
        sppr_sd = succ_sd;
    }

    // Print results
    printf("Method random_global:        %ld/%d wins/games = %ld %% total time = %lf ms | average time = %lf\n", win_r, games,(win_r*100)/games, rand_time, rand_time/games);
    printf("Method random_drawer:        %ld/%d wins/games = %ld %% total time = %lf ms | average time = %lf\n", win_rd, games,(win_rd*100)/games, rand_drawer_time, rand_drawer_time/games);
    printf("Method smart_global:         %ld/%d wins/games = %ld %% total time = %lf ms | average time = %lf\n", win_s, games,(win_s*100)/games, seq_time, seq_time/games);
    printf("Method smart_drawer:         %ld/%d wins/games = %ld %% total time = %lf ms | average time = %lf\n", win_sd, games,(win_sd*100)/games, smart_drawer_time, smart_drawer_time/games);

    return EXIT_SUCCESS;
}

// Better rand() function found online since rand() is too predictable
int better_rand(void) {
    random_ = random_ * 1103515245 + 12345;
    return ((unsigned)(random_ / 65536) % 32768);
}
// Second needed function for better_rand
void better_rand_two(unsigned seed) {
    random_ = seed;
}

// Global locking random drawer strategy
void* random_global(void *arg) {
    int i = *(int *)arg;
    
    pthread_mutex_lock(&global_mutex); // Lock the global mutex
    rand_search(i, drawer);  // Each prisoner searches in rand drawer
    pthread_mutex_unlock(&global_mutex); // Unlock the global mutex
    free(arg);
    return NULL;
}

// Generating a random arrangement of prisoners' IDs in the drawers.
void rand_gen(drawer_t draw[NO_OF_DRAWER]) {
    for (int i = 0; i < NO_OF_DRAWER; ++i)
        draw[i].pid = i + 1;

    for (int i = NO_OF_DRAWER - 1; i > 0; --i) {
        int j = better_rand() % (i + 1);
        int temp = draw[i].pid;
        draw[i].pid = draw[j].pid;
        draw[j].pid = temp;
    }
}

// Random drawer strategy with individual drawer locks
void rand_search(int n, drawer_t draw[NO_OF_DRAWER]) {
    int count = 1;
    while (count <= 50) {
        int i = better_rand() % 100;
        pthread_mutex_lock(&draw[i].mutex);
        if (n + 1 == draw[i].pid) {
            ++succ_rd;
            pthread_mutex_unlock(&draw[i].mutex);
            return;
        }
        pthread_mutex_unlock(&draw[i].mutex);
        count++;
    }
}

void* random_drawer(void *arg) {
    int i = *(int *)arg;
    rand_search(i, drawer);
    free(arg);
    return NULL;
}

// Sequential drawer strategy with global lock
void smart_search(int n, drawer_t draw[NO_OF_DRAWER]) {
    int count = 1;
    int i = n;
    while (count <= 50) {
        pthread_mutex_lock(&draw[i].mutex);
        if (n + 1 == draw[i].pid) {
            ++succ_s;
            pthread_mutex_unlock(&draw[i].mutex);
            return;
        }
        pthread_mutex_unlock(&draw[i].mutex);
        i = draw[i].pid - 1;
        count++;
    }
}
// Smart drawer strategy with global locks
void* smart_global(void *arg) {
    int i = *(int *)arg;
    pthread_mutex_lock(&global_mutex);
    smart_search(i, drawer);
    pthread_mutex_unlock(&global_mutex);
    free(arg);
    return NULL;
}

// Smart drawer strategy with individual locks
void* smart_drawer(void *arg) {
    int index = *(int *)arg;
    int count = 1;
    int i = index;
    while (count <= 50) {
        pthread_mutex_lock(&drawer[i].mutex);
        if (index + 1 == drawer[i].pid) {
            ++succ_sd;
            pthread_mutex_unlock(&drawer[i].mutex);
            break;
        }
        int next_idx = drawer[i].pid - 1;
        pthread_mutex_unlock(&drawer[i].mutex);
        i = next_idx;
        count++;
    }
    free(arg);
    return NULL;
}

void run_sim(int n, void* (*proc)(void *)) {
    for (unsigned int i = 0; i < n; i++) {
        int* a = malloc(sizeof(int));
        *a = i;
        pthread_create(&prisoner[i], NULL, proc, a);
    }

    for (unsigned int i = 0; i < n; i++) {
        pthread_join(prisoner[i], NULL);
    }
}

static double timeit(int n, void* (*proc)(void *)) {
    clock_t t1 = clock();
    run_sim(n, proc);
    clock_t t2 = clock();
    return ((double)(t2 - t1)) / CLOCKS_PER_SEC * 1000;
}
