#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

/*
Demonstration of Barriers File 2: Condition variable barrier
This demonstrates and tests (in terms of performance) a barrier
made with condition variables
*/

typedef struct{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int count;
    long long int loops;
    int num_threads;
}Barrier;


void* worker(void* args);

int main(int argc, char** argv){
    if (argc != 3){
        fprintf(stderr, "Run this as: %s <num_threads> <loops (n:10^n)>\n", argv[0]);
        return 1;
    }
    struct timespec start,end;
    double time_elapsed;
    int num_threads = atoi(argv[1]);
    long long int loops = pow(10,atoi(argv[2]));
    
    Barrier barrier;
    
    pthread_mutex_init(&barrier.lock, NULL);
    pthread_cond_init(&barrier.cond, NULL);
    barrier.count = 0;
    barrier.num_threads = num_threads;
    barrier.loops = loops;
    pthread_t threads[num_threads];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_threads; i++){
        pthread_create(&threads[i], NULL, worker, &barrier);
    }
    for (int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %lf seconds\n", time_elapsed);
    pthread_mutex_destroy(&barrier.lock);
    pthread_cond_destroy(&barrier.cond);
    return 0;
}

void* worker(void* args){
    Barrier* barrier = (Barrier*)args;
    for (int i = 0; i < barrier->loops; i++){
        pthread_mutex_lock(&barrier->lock);
        barrier->count++;
        if (barrier->count == barrier->num_threads){
            barrier->count = 0;
            pthread_cond_broadcast(&barrier->cond);
        }
        else{
            pthread_cond_wait(&barrier->cond, &barrier->lock);
        }
        pthread_mutex_unlock(&barrier->lock);
    }
    return NULL;
}