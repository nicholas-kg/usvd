#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Shared global counter updated by all threads
long long count = 0;
long long iterations = 0;

// Synchronization primitives for mutual exclusion and read-write locking
pthread_mutex_t mutex;
pthread_rwlock_t rwlock;

// Thread worker function prototypes
void* worker_mutex(void* arg);
void* worker_rwlock(void* arg);
void* worker_atomic(void* arg);

int main(int argc, char* argv[]){
    if(argc!=4){
        fprintf(stderr,"Usage: %s<mode: 1=mutex, 2=rwlock, 3=atomic> <num_threads> <iterations>\n", argv[0]);
        return 1;
    }

    // Parse command-line arguments
    int mode = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    iterations = atoll(argv[3]);

    if (num_threads <= 0 || iterations < 0 || mode < 1 || mode > 3) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    // Select the appropriate worker function and initialize the corresponding lock
    void* (*worker_func)(void*) = NULL;
    if (mode == 1) {
        pthread_mutex_init(&mutex, NULL);
        worker_func = worker_mutex;
    } else if (mode == 2) {
        pthread_rwlock_init(&rwlock, NULL);
        worker_func = worker_rwlock;
    } else {
        worker_func = worker_atomic;
    }

    // Allocate memory for thread handles
    pthread_t* threads = malloc((size_t)num_threads * sizeof(pthread_t));
    if(!threads){
        perror("malloc");
        return 1;
    }

    count = 0;
    struct timespec start,end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_func, NULL);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;

    // Calculate deterministic expected value
    long long expected = (long long)num_threads * iterations;

    printf("%d,%d,%lld,%lld,%lld,%.6f\n",
           mode, num_threads, iterations, count, expected, time_elapsed);

    // Clean up allocated synchronization primitives 
    if (mode == 1) pthread_mutex_destroy(&mutex);   
    if (mode == 2) pthread_rwlock_destroy(&rwlock);
    free(threads);

    // Return 0 if the final counter matches the expected deterministic value, 1 otherwise
    return (count == expected) ? 0 : 1;

}

//Synchronize increments using a standard mutex lock
void* worker_mutex(void* arg) {
    (void)arg;
    for (long long i = 0; i < iterations; i++) {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

//Synchronize increments using a read-write lock
void* worker_rwlock(void* arg) {
    (void)arg;
    for (long long i = 0; i < iterations; i++) {
        pthread_rwlock_wrlock(&rwlock);
        count++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

//Synchronize increments using lock-free atomic built-ins
void* worker_atomic(void* arg) {
    (void)arg;
    for (long long i = 0; i < iterations; i++) {
        __atomic_fetch_add(&count, 1, __ATOMIC_SEQ_CST);
    }
    return NULL;
}
