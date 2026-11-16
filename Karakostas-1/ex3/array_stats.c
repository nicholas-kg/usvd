#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NUM_THREADS 4

typedef struct{
    int rank;
    int* array;
} ThreadArgs;

typedef struct {
    long long int nr_of_elements_equal_to_zero;
    long long int nr_of_elements_equal_to_one;
    long long int nr_of_elements_greater_than_one;
} array_stats;

array_stats all_stats[NUM_THREADS];

void Array_init(int* A, int size);
void sequential_stats(int **arrays, int size);
void print_stats();
void* parallel_stats(void* args);

int main(int argc, char** argv){
    struct timespec start,end;
    double time_elapsed;
    srand(time(NULL));
    if (argc != 2){
        fprintf(stderr, "Call this as %s <array size>\n", argv[0]);;
        return 1;
    }
    int size = atoi(argv[1]);
    int **arrays = malloc(4*sizeof(int*));
    if (!arrays){
        fprintf(stderr, "Error during allocation\n");
        return 1;
    }
    for (int i = 0; i < 4; i++){
        Array_init(arrays[i], size);
        if (!arrays[i]){
            free(arrays);
            fprintf(stderr, "Error during allocation\n");
            return 1;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    sequential_stats(arrays, size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    print_stats();
    printf("Sequential implementation lasted: %lf seconds\n", time_elapsed)
    //Start initializing all things needed for the parallel implementation
    ThreadArgs* args = malloc(NUM_THREADS*sizeof(ThreadArgs));
    pthread_t threads[NUM_THREADS];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++){
        args[i].rank = i;
        args[i].array = arrays[i];
        pthread_create(&threads[i], NULL, parallel_stats, &args[i]);
    } 
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    print_stats();
    printf("Parallel implementation lasted: %lf seconds\n", time_elapsed);
    //cleanup
    for (int i = 0; i < 4; i++){
        free(arrays[i]);
    }
    free(args);
    free(arrays);
    return 0;
}

void Array_init(int* A, int size){
    A = malloc(size*sizeof(int));
    if (!A){
        fprintf(stderr, "Error During allocation\n");
        return 1;
    }
    for (int i = 0; i < size; i++) A[i] = rand() % RAND_MAX;
}

void sequential_stats(int **arrays, int size){
    for (int i = 0; i < 4; i++){
        all_stats[i].nr_of_elements_equal_to_one = 0;
        all_stats[i].nr_of_elements_equal_to_zero = 0;
        all_stats[i].nr_of_elements_greater_than_one = 0;
        for (int j = 0; j < size; j++){
            if (arrays[i][j] == 1) all_stats[i].nr_of_elements_equal_to_one++;
            else if (arrays[i][j] == 0) all_stats[i].nr_of_elements_equal_to_zero++;
            else all_stats[i].nr_of_elements_greater_than_one++;
        }
    }
}

void print_stats(){
    printf("Array stats\n");
    for (int i = 0; i < 4; i++){
        printf("Stats for array %d\n", i);
        printf("Number of elements equal to 0: %lld", all_stats[i].nr_of_elements_equal_to_zero);
        printf("Number of elements equal to 1: %lld", all_stats[i].nr_of_elements_equal_to_one);
        printf("Number of elements greater than 1: %lld", all_stats[i].nr_of_elements_greater_than_one);
    }
}


void* parallel_stats(void* args){
    ThreadArgs* argin = (ThreadArgs*)args;
    int my_rank = argin->rank;
    int* target_array = argin->array;
    int my_zeros = 0;
    int my_ones = 0;
    int my_greater = 0;
    for (int i = 0; i < sizeof(target_array)/sizeof(int); i++){
        if (target_array[i] == 1) my_ones++;
        else if (target_array[i] == 0) my_zeros++;
        else my_greater++;
    }
    all_stats[my_rank].nr_of_elements_equal_to_one = my_ones;
    all_stats[my_rank].nr_of_elements_equal_to_zero = my_zeros;
    all_stats[my_rank].nr_of_elements_greater_than_one = my_greater;
    return NULL;
}
