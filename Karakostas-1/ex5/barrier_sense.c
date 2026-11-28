#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    pthread_mutex_t lock;
    int barrier_count;
    int thread_count;
    bool flag;
} Barrier;

static Barrier barrier;
static long long iterations = 0;

void barrier_init(Barrier *bar, pthread_mutexattr_t *attr, int count);
void barrier_wait(Barrier *bar, bool *local_flag);
void barrier_destroy(Barrier *bar);
void *thread_func(void *arg);


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <iterations>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    iterations = atoll(argv[2]);

    if (num_threads <= 0 || iterations < 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    barrier_init(&barrier, NULL, num_threads);

    pthread_t *thread_ids = malloc((size_t)num_threads * sizeof(pthread_t));
    if (!thread_ids) {
        perror("malloc");
        barrier_destroy(&barrier);
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread_ids[i], NULL, thread_func, NULL);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Sense-Reversal Centralized Barrier | Threads: %d | Iterations: %lld | Time: %.6f sec\n",
           num_threads, iterations, elapsed);

    barrier_destroy(&barrier);
    free(thread_ids);
    return 0;
}

void barrier_init(Barrier *bar, pthread_mutexattr_t *attr, int count) {
    pthread_mutex_init(&(bar->lock), attr);
    bar->thread_count = 0;
    bar->barrier_count = count;
    bar->flag = false;
}

void barrier_wait(Barrier *bar, bool *local_flag) {
    pthread_mutex_lock(&(bar->lock));
    bar->thread_count++;
    *local_flag = !(*local_flag);

    if (bar->thread_count == bar->barrier_count) {
        bar->thread_count = 0;
        bar->flag = *local_flag;
        pthread_mutex_unlock(&(bar->lock));
    } else {
        pthread_mutex_unlock(&(bar->lock));
        while (bar->flag != *local_flag) {
            // busy-wait for flag 
        }
    }
}

void barrier_destroy(Barrier *bar) {
    pthread_mutex_destroy(&(bar->lock));
}

void *thread_func(void *arg) {
    (void)arg;
    bool local_flag = false;

    for (long long i = 0; i < iterations; i++) {
        barrier_wait(&barrier, &local_flag);
    }
    return NULL;
}