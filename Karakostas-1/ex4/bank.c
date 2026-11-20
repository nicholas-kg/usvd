#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//Will need a struct to store the fine-grained lock in the account array
//Possible candidate -> struct{balance, mutex lock}

//Enum for simplicity
typedef enum {FINE_GRAINED, COARSE_GRAINED} lock_type;

int main(int argc, char** argv){
    if (argc != 6){
        fprintf(stderr, "Run this as: 
        %s <account's num> <num of transactions> <percent of inquiry transactions> <lock type> <num_threads> \n", argv[0]);
        return 1;
    }
    int accounts_num = atoi(argv[1]);
    int transactions = atoi(argv[2]);
    int inquiry_percent = atoi(argv[3]);
    if (inquiry_percent > 100 || inquiry_percent < 0){
        fprintf(stderr, "Percent outside the 0-100 range\n");
        return 1;
    }
    int lock = atoi(argv[4]);
    if (lock != 0 || lock != 1){
        fprintf(stderr, "Choose between lock type 0 (fine-grained) or lock type 1 (coarse-grained)\n");
        return 1;
    }
    int num_threads = atoi(argv[5]);
    //if statement to determine type of data stored in the array
    //There will be 2 different parallel functions based on the lock type
    return 0;
}