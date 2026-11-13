#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//All matrixes used in this program have size N x N
void matrix_mult(double** first, double** second, double** result, int N);
void swap_matrix(double** first, double** second, int N);
void matrix_pow(double** A, double** out, int N, int pow);
void print_matrix(double** A, int N);


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