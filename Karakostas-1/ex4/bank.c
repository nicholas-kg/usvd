#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
//Will need a struct to store the fine-grained lock in the account array
//Possible candidate -> struct{balance, mutex lock}
typedef struct{
    long long int balance;
    pthread_mutex_t fine_mutex;
}Account;
//Enum for simplicity
typedef enum {FINE_GRAINED, COARSE_GRAINED} lock_type;
//Lock for the coarse-grained version
pthread_mutex_t coarse_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct{
    //Leave one of the two be NULL (Depends on lock type)
    long long int* accounts_1;
    Account* accounts_2;
    int accounts_num;
    int transactions;
    int inquiries; //0-100 (dictifies a percentage)
}Args;

void* bank_worker_0(void* args);
void* bank_worker_1(void* args);


int main(int argc, char** argv){
    struct timespec start,end;
    double time_elapsed;
    srand(time(NULL));
    if (argc != 6){
        fprintf(stderr, "Run this as:  %s <account's num> <num of transactions per thread> <percent of inquiry transactions> <lock type> <num_threads> \n", argv[0]);
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
    if (lock != 0 && lock != 1){
        fprintf(stderr, "Choose between lock type 0 (fine-grained) or lock type 1 (coarse-grained)\n");
        return 1;
    }
    int num_threads = atoi(argv[5]);
    //if statement to determine type of data stored in the array
    //There will be 2 different parallel functions based on the lock type
    //Case lock = 0
    if (lock == 0){
        long long int *accounts = malloc(accounts_num*sizeof(long long int*));
        if (!accounts){
            fprintf(stderr, "Error during allocation\n");
            return 1;
        }
        //Checksum to ensure no money lost 
        int checksum = 0;
        for (int i = 0; i < accounts_num; i++){
            //set a lower bound for the balance of each account (here 100)
            accounts[i] = rand() % 100000 ;
            checksum += accounts[i];
        }
        Args* thread_args = malloc(num_threads*sizeof(Args));
        pthread_t threads[num_threads];
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < num_threads; i++){
            thread_args[i].accounts_1 = accounts;
            thread_args[i].accounts_2 = NULL;
            thread_args[i].transactions = transactions;
            thread_args[i].inquiries = inquiry_percent;
            pthread_create(&threads[i], NULL, bank_worker_0, &thread_args[i]);
        }
        for (int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Time elapsed for coarse-grained lock implementation: %lf\n", time_elapsed);
        int check;
        for (int i = 0; i < accounts_num; i++){
            check += accounts[i];
        }
        if(checksum != check){
            fprintf(stderr, "Did not retain total funds in accounts. Program failed\n");
            return 1;
        }
    }
    else{
        //Fine grained 
        Account* accounts = malloc(accounts_num*sizeof(Account));
        if (!accounts) {
            fprintf(stderr, "Error during accounts allocation\n");
            return 1;
        }

        int checksum = 0;
        for (int i = 0; i < accounts_num; i++){
            accounts[i].balance = rand() % 100000; 
            checksum += accounts[i].balance;
        }

        Args* thread_args = malloc(num_threads*sizeof(Args));
        pthread_t threads[num_threads];
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < num_threads; i++){
            thread_args[i].accounts_1 = NULL;
            thread_args[i].accounts_2 = accounts;
            thread_args[i].transactions = transactions;
            thread_args[i].inquiries = inquiry_percent;
            pthread_create(&threads[i], NULL, bank_worker_1, &thread_args[i]);
        }
        for (int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;

        int check = 0;
        for (int i = 0; i < accounts_num; i++) {
            check += accounts[i].balance;
        }

        if(checksum != check){
            fprintf(stderr, "Did not retain total funds in accounts. Program failed\n");
            return 1;
        }
        free(accounts);
    }
}

void* bank_worker_0(void* args){
    Args* argsin = (Args*)args;
    int inq_transactions = ceil(argsin->transactions * (argsin->inquiries/100));
    int transfers = argsin->transactions - inq_transactions;
    long long int* accounts = argsin->accounts_1;
    int count = argsin->accounts_num;
    //Randomly choose transaction type in each loop
    //Randomly choose account to perform transaction on 
    for (int i = 0; i < argsin->transactions; i++){
        int type;
        if (inq_transactions == 0) type = 1;
        else if (transfers == 0) type = 2;
        else type = rand() % 2;
        int target = rand() % count;
        if(type == 1){//transfer
            pthread_mutex_lock(&coarse_lock);
            int recipient;
            do{
                recipient = rand() % count;
                if (accounts[target] == 0) target = rand() % count;
            }while(recipient == target || accounts[target] == 0);
            long long int to_move = rand() % accounts[target];
            accounts[recipient] += to_move;
            accounts[target] -= to_move;
            pthread_mutex_unlock(&coarse_lock);
            transfers--;
        }
        else{//inquiry
            pthread_mutex_lock(&coarse_lock);
            printf("Account %d has balance: %lld\n", target, accounts[target]);
            pthread_mutex_unlock(&coarse_lock);
            inq_transactions--;
        } 
    }
    return NULL;
}

void* bank_worker_1(void* args){
    Args* argsin = (Args*)args;

    int inq_transactions = ceil(argsin->transactions * (argsin->inquiries/100));
    int transfers = argsin->transactions - inq_transactions;

    Account* accounts = argsin->accounts_2;
    int count = argsin->accounts_num;

    for(int i=0; i<argsin->transactions; i++){
        int type;
        if (inq_transactions == 0) type = 1;
        else if (transfers == 0) type = 2;
        else type = rand() % 2;

        int target = rand() % count;

        if(type == 1){  
            int recipient;
            do{
                recipient = rand()%count;
            }while(recipient==target);

            // lock in ascending index order to prevent deadlocks
            int first = (target<recipient)? target : recipient;
            int second = (target<recipient)? recipient : target;

            pthread_mutex_lock(&accounts[first].fine_mutex);
            pthread_mutex_lock(&accounts[second].fine_mutex);

            if (accounts[target].balance > 0) {
                long long int to_move = rand() % accounts[target].balance;
                accounts[recipient].balance += to_move;
                accounts[target].balance -= to_move;
            }
            pthread_mutex_unlock(&accounts[second].fine_mutex);
            pthread_mutex_unlock(&accounts[first].fine_mutex);

            transfers--;
        }else{
            pthread_mutex_lock(&accounts[target].fine_mutex);
            printf("Account %d has balance: %lld\n", target, accounts[target]);
            pthread_mutex_unlock(&accounts[target].fine_mutex);
            inq_transactions--;
        }
    }
    return NULL;
}
