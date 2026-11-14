#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Arguments needed for threads used in the parallelization of this 
//algorithm
typedef struct{
    double **A;
    double **B;
    double **C;
    int N;
    int rank;
    int thread_count;
}Args;

//All matrixes used in this program have size N x N
void matrix_mult(int N, double** first, double** second, double** result);
void matrix_pow(double** A, double** out, int pow, int N);
void* parallel_mult(void* args);
void parallel_pow(double** A, double** out, int N, int pow,pthread_t* threads, int thread_count);


int main(int argc, char** argv){
    struct timespec start,end;
    double time_elapsed;
    srand(time(NULL));
    if (argc != 4){
        fprintf(stderr, "Call this program as: %s <size of matrix> <power> <num of threads>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int p = atoi(argv[2]);
    int thread_count = atoi(argv[3]);

    clock_gettime(CLOCK_MONOTONIC, &start);
    double** A = malloc(N*sizeof(double*));
    double** out = malloc(N*sizeof(double*));
    if (!A || !out){
        fprintf(stderr, "Error during matrix allocation\n");
        return 1;
    }
    //Complete allocation and initialize the matrix to store normalized 
    //values in range [0,1]
    for (int i = 0; i < N; i++){
        A[i] = malloc(N*sizeof(double));
        out[i] = malloc(N*sizeof(double));
        if (!A[i] || !out[i]){
            fprintf(stderr, "Error during matrix allocation\n");
            if (A[i]) free(A);
            if (out[i]) free(out);
            free(A);
            free(out);
            return 1;    
        }
        for (int j = 0; j < N; j++){
            A[i][j] = rand() % RAND_MAX;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed during matrix allocation and initialization: %lf\n", time_elapsed);

    //Run non parallel algorithm 
    clock_gettime(CLOCK_MONOTONIC, &start);
    matrix_pow(A,out,p,N);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Result of non parallel implementation\n\n");
    printf("\n");
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %lf seconds\n", time_elapsed);
    
    //Now for the parallel Implementation
    //Create the threads and initialize the barrier
    //And then call the parallel function
    pthread_t threads[thread_count];
    //Args will be initialized in the parallel function (see code bellow)
    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_pow(A,out,N,p,threads,thread_count);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Result of parallel implementation\n\n");
    printf("\n");
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed for the prallel implementation: %lf seconds\n", time_elapsed);

    //Mem cleanup
    for (int i = 0; i < N; i++){
        if (A[i]) free(A[i]);
        if (out[i]) free(out[i]);
    }
    free(A);
    free(out);
    return 0;
}

void matrix_mult(int N, double** first, double** second, double** result){
    if (N <= 1){
        fprintf(stderr, "Matrix has to have size > 1\n");
        exit(1);
    }
    double tmp[N][N];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tmp[i][j] = 0;
            for (int k = 0; k < N; k++) {
                tmp[i][j] += first[i][k] * second[k][j];
                result[i][j] = tmp[i][j];
            }
        }            
    }    
}

void matrix_pow(double** A, double** out, int pow, int N){
    if (pow <= 0){
        fprintf(stderr, "Power has to be a positive integer\n");
        exit(1);
    }
    if (N <= 1){
        fprintf(stderr, "Matrix has to have size > 1\n");
        exit(1);
    }
    //Allocation and sanity checks
    double** res = malloc(N*sizeof(double*));
    double** tmp = malloc(N*sizeof(double*));
    double** base = malloc(N*sizeof(double*));
    if (!res || !tmp || !base){
        if(res) free(res);
        if(base) free(base);
        if(tmp) free(tmp);
    }
    for (int i = 0; i < N; i++){
        res[i] = malloc(N * sizeof(double));
        tmp[i] = malloc(N * sizeof(double));
        base[i] = malloc(N * sizeof(double));
        if (!res[i] || !tmp[i] || !base[i]){
            if (base) {
            for (int i = 0; i < N; i++) {
                free(base[i]); // free(NULL) is safely ignored in C
            }
            free(base);
            }
            if (tmp) {
                for (int i = 0; i < N; i++) {
                free(tmp[i]);
                }
                free(tmp);
            }
            if (res) {
            for (int i = 0; i < N; i++) {
                free(res[i]);
            }
            free(res);
            }
        }
    }
    //Initialization of res and base
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            res[i][j] = (i==j) ? 1 : 0;
            base[i][j] = A[i][j];
        }
    }
    while (pow > 0){
        //p is odd -> tmp=res*base
        if (pow % 2){
            matrix_mult(N, res, base, tmp);
            //Swap res and tmp 
            double** temp = res;
            res = tmp;
            tmp = temp;
        }
        //res = base*base
        if (pow > 1){
            matrix_mult(N, base, base, tmp);
            //swap base and tmp
            double** temp = base;
            base = tmp;
            tmp = temp;
        }
        pow = pow / 2;   
    }
    //Result copied to out 
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            out[i][j] = res[i][j];
        }
    }
}

void* parallel_mult(void* args){
    Args* argin = (Args*)args;
    //Portion of the matrix assigned to this thread.
    int start = argin->rank*argin->N/argin->thread_count;
    int end = start+argin->N/argin->thread_count;
    for (int i = start; i < end; i++){
        for (int j = 0; j < argin->N; j++){
            argin->C[i][j] = 0;
            for (int k = 0; k < argin->N; k++){
                argin->C[i][j] += argin->A[i][k] * argin->B[k][j];                
            }
        }
    }
    return NULL;
}

void parallel_pow(double** A, double** out, int N, int pow, pthread_t* threads, int thread_count){
    //Initialize args for thread workers
    if (pow <= 0){
        fprintf(stderr, "Power has to be a positive integer\n");
        exit(1);
    }
    if (N <= 1){
        fprintf(stderr, "Matrix has to have size > 1\n");
        exit(1);
    }
    Args* args = malloc(sizeof(Args));
    if (!args){
        fprintf(stderr, "Error during matrix allocation\n");
        exit(1);
    }
    args->N = N;
    args->thread_count = thread_count;
    double** res = malloc(N*sizeof(double*));
    double** tmp = malloc(N*sizeof(double*));
    if (!res || !tmp){
        fprintf(stderr, "Error during matrix allocation\n");
        if (res) free(res);
        if (tmp) free(tmp);
        exit(1);
    }
    for (int i = 0; i < N; i++){
        res[i] = malloc(N*sizeof(double));
        tmp[i] = malloc(N*sizeof(double));
    }
    double** base = A;
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            res[i][j] = (i==j) ? 1 : 0;
        }
    }
    while (pow > 0){
        if (pow % 2){
            args->A = res;
            args->B = base;
            args->C = tmp;
            for (int i = 0; i < thread_count; i++){
                args->rank = i;
                pthread_create(&threads[i], NULL, parallel_mult, args);
            }
            for (int i = 0; i < thread_count; i++){
                pthread_join(threads[i], NULL);
            }
            double** temp = res;
            res = tmp;
            tmp = temp;
        }
        if (pow > 1){
            args->A = base;
            args->B = base;
            args->C = tmp;
            for (int i = 0; i < thread_count; i++){
                args->rank = i;
                pthread_create(&threads[i], NULL, parallel_mult, args);
            }
            for (int i = 0; i < thread_count; i++){
                pthread_join(threads[i], NULL);
            }
            double** temp = base;
            base = tmp;
            tmp = temp;
        }
        pow = pow / 2;
    }
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            out[i][j] = res[i][j];
        }
    }
    for (int i = 0; i < N; i++){
        free(tmp[i]);
        free(res[i]);
    }
    free(args);
}