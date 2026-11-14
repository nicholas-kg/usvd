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
    int rank
    int thread_count;
}Args;

//All matrixes used in this program have size N x N
void matrix_mult(double** first, double** second, double** result, int N);
void swap_matrix(double** first, double** second, int N);
void matrix_pow(double** A, double** out, int N, int pow);
void print_matrix(double** A, int N);
void* parallel_mult(Args* args);
void parallel_pow(double** A, double** out, int N, int pow,pthread_t* threads, int thread_count);


int main(int argc, char** argv){
    struct timespec start,end;
    double time_elapsed;
    srand(time(NULL));
    if (argc != 4){
        fprintf(stderr, "Call this program as: %s <size of matrix> <power> <num of threads>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    p = atoi(argv[2]);
    thread_count = atoi(argv[3]);

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
    matrix_pow(A,out,N,pow);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Result of non parallel implementation\n\n");
    print_matrix(out);
    printf("\n");
    time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %lf seconds\n", time_elapsed);
    
    //Now for the parallel Implementation
    //Create the threads and initialize the barrier
    //And then call the parallel function
    pthread_t* threads[thread_count];
    //Args will be initialized in the parallel function (see code bellow)
    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_pow(A,out,N,pow,threads,thread_count);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Result of parallel implementation\n\n");
    print_matrix(out);
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

void matrix_mult(double** first, double** second, double** result, int N){
    if (p <= 0){
        fprintf(stderr, "Power has to be a positive integer\n");
        exit(1);
    }
    if (N <= 1){
        fprintf(stderr, "Matrix has to have size > 1\n");
        exit(1);
    }
    tmp[N][N];
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

void swap_matrix(double** first, double** second, int N){
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            double temp = first[i][j];
            first[i][j] = second[i][j];
            second[i][j] = temp;
        }
    }    
}


void matrix_pow(double** A, double** out, int N, int pow){
    if (p <= 0){
        fprintf(stderr, "Power has to be a positive integer\n");
        exit(1);
    }
    if (N <= 1){
        fprintf(stderr, "Matrix has to have size > 1\n");
        exit(1);
    }

    double res[N][N];
    double tmp[N][N];
    double base[N][N];

    //Initialization of res and base
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            res[i][j] = (i==j) ? 1 : 0;
            base[i][j] = A[i][j];
        }
    }
    while (p > 0){
        //p is odd -> tmp=res*base
        if (p % 2){
            matrix_mult(res, base, tmp, N);
            //Swap res and tmp 
            swap_matrix(res,tmp);
        }
        //res = base*base
        if (p > 1){
            matrix_mult(base, base, res, N);
            //swap base and tmp
            swap_matrix(base,tmp);
        }
        p = p / 2;   
    }
    //Result copied to out 
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            out[i][j] = res[i][j];
        }
    }
}

void print_matrix(double** A, int N){
    for (int i = 0; i < N; i++){
        printf("| ");
        for (int j = 0; j < N; j++){
            printf("%lf ", A[i][j]);
        }
        printf("|\n");
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

void parallel_pow(double** A, double** out, int N, int pow,pthread_t* threads, int thread_count){
    //Initialize args for thread workers
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
    while (p > 0){
        if (p % 2){
            args->A = res;
            args->B = base;
            args->C = tmp;
            for (int i = 0; i < thread_count; i++){
                args->rank = i
                pthread_create(threads[i], NULL, parallel_mult, args);
            }
            for (int i = 0; i < thread_count; i++){
                pthread_join(threads[i], NULL);
            }
            double** temp = res;
            res = tmp;
            tmp = temp;
        }
        if (p > 1){
            args->A = base;
            args->B = base;
            args->C = res;
            for (int i = 0; i < thread_count; i++){
                args->rank = i
                pthread_create(threads[i], NULL, parallel_mult, args);
            }
            for (int i = 0; i < thread_count; i++){
                pthread_join(threads[i], NULL);
            }
            double** temp = base;
            base = tmp;
            tmp = temp;
        }
        p = p / 2;
    }
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            out[i][j] = res[i][j];
        }
    }
    for (int i = 0; i < r; i++){
        free(tmp[i]);
        free(res[i]);
    }
    free(args);
}