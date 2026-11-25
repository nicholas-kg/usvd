#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

/*
Demonstration of Barriers File 1: The pthread_barrier
This demonstrates and tests (in terms of performance) the barrier
defined in pthread.h
*/

pthread_barrier_t barrier;

typedef struct{
    /*Array to validate that barrier works*/
    long long int* results;
    long long int loops;
    int id;
    int num_threads;
}ThreadArgs;

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
    pthread_barrier_init(&barrier, NULL, num_threads);
    long long int* results = calloc(num_threads, sizeof(long long int));
    if (!results){
        fprintf(stderr, "Error During allocation\n");
        return 1;
    }
    ThreadArgs* args = malloc(num_threads*sizeof(ThreadArgs));
    if (!results){
        fprintf(stderr, "Error During allocation\n");
        return 1;
    }
    pthread_t threads[num_threads];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_threads; i++){
        args[i].id = i;
        args[i].num_threads = num_threads;
        args[i].loops = loops;
        args[i].results = results;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %lf seconds\n", time_elapsed);
    pthread_barrier_destroy(&barrier);
    free(args);
    free(results);
    return 0;
}

void* worker(void* args){
    /* 
        Uncomment the bellow commented lines of code
        to demonstrate the barrier works
    */
    ThreadArgs* argsin = (ThreadArgs*)args;
    long long int loops = argsin->loops;
    // int threads = argsin->num_threads;
    // int rank = argsin->id;
    // long long int* results = argsin->results;
    for (int i = 0; i < loops; i++){
        
        

        // results[rank] = i;
        pthread_barrier_wait(&barrier);
        // // Ensure barrier works
        // for (int j = 0; j < threads; j++){
        //     assert(results[j] == i);
        // }
        // pthread_barrier_wait(&barrier);
    }
    return NULL;
}