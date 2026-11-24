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
    pthread_rwlock_t fine_rwlock;
}Account;
//Enum for simplicity
typedef enum {COARSE_MUTEX =0 ,COARSE_RWLOCK=1 ,FINE_MUTEX=2 , FINE_RWLOCK=3} lock_type;
//Lock for the coarse-grained version
pthread_mutex_t coarse_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t coarse_rwlock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct{
    //Leave one of the two be NULL (Depends on lock type)
    long long int* accounts_1;
    Account* accounts_2;
    int accounts_num;
    int transactions;
    int inquiries; //0-100 (dictifies a percentage)
    lock_type mode;
    long long int priv_sum; //sum of balances during inquiries
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
    // argument parsing
    int accounts_num = atoi(argv[1]);
    int transactions = atoi(argv[2]);
    int inquiry_percent = atoi(argv[3]);
    if (inquiry_percent > 100 || inquiry_percent < 0){
        fprintf(stderr, "Percent outside the 0-100 range\n");
        return 1;
    }
    int lock = atoi(argv[4]);
    if (lock < 0 || lock >3 ){
        fprintf(stderr, "Choose lock type: 0 (coarse_mutex), 1 (coarse-rwlock), 2 (fine_mutex), 3 (fine_rwlock)\n");
        return 1;
    }
    int num_threads = atoi(argv[5]);

    // total bank balance sum variables
    //check == sum after transactions
    long long int checksum = 0;
    long long int check = 0;

    Args* thread_args = malloc(num_threads*sizeof(Args));
    pthread_t threads[num_threads];
    //if statement to determine type of data stored in the array
    //There will be 2 different parallel functions based on the lock type

    //Coarse-grained lock type(MUTEX OR RWLOCK)
    if (lock == 0 || lock==1){
        long long int *accounts = malloc(accounts_num*sizeof(long long int));
        if (!accounts){
            fprintf(stderr, "Error during allocation\n");
            free(thread_args);
            return 1;
        }
        //Checksum to ensure no money lost 
        for (int i = 0; i < accounts_num; i++){
            //set a lower bound for the balance of each account (here 100)
            accounts[i] = rand() % 100000 ;
            checksum += accounts[i];
        }
       
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < num_threads; i++){
            thread_args[i].accounts_1 = accounts;
            thread_args[i].accounts_2 = NULL;
            thread_args[i].accounts_num = accounts_num;
            thread_args[i].transactions = transactions;
            thread_args[i].inquiries = inquiry_percent;
            thread_args[i].mode = (lock_type)lock;
            thread_args[i].priv_sum = 0;
            pthread_create(&threads[i], NULL, bank_worker_0, &thread_args[i]);
        }
        for (int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;

        for (int i = 0; i < accounts_num; i++){
            check += accounts[i];
        }
    
        free(accounts);
    }
    else{
        //Fine-grained lock type(MUTEX OR RWLOCK) 
        Account* accounts = malloc(accounts_num*sizeof(Account));
        if (!accounts) {
            fprintf(stderr, "Error during accounts allocation\n");
            free(thread_args);
            return 1;
        }

        for (int i = 0; i < accounts_num; i++){
            accounts[i].balance = rand() % 100000; 
            pthread_mutex_init(&accounts[i].fine_mutex, NULL);
            pthread_rwlock_init(&accounts[i].fine_rwlock, NULL);
            checksum += accounts[i].balance;
        }

        
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < num_threads; i++){
            thread_args[i].accounts_1 = NULL;
            thread_args[i].accounts_2 = accounts;
            thread_args[i].accounts_num = accounts_num;
            thread_args[i].transactions = transactions;
            thread_args[i].inquiries = inquiry_percent;
            thread_args[i].mode = (lock_type)lock;
            thread_args[i].priv_sum = 0;
            pthread_create(&threads[i], NULL, bank_worker_1, &thread_args[i]);
        }
        for (int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) / 1e9;

        for (int i = 0; i < accounts_num; i++) {
            check += accounts[i].balance;
            pthread_mutex_destroy(&accounts[i].fine_mutex);
            pthread_rwlock_destroy(&accounts[i].fine_rwlock);
        }

        free(accounts);
    }

    // total sum of inquiries of every thread
    long long int total_inquiries = 0;
    for(int i=0; i<num_threads; i++){
        total_inquiries+=thread_args[i].priv_sum;
    }

    if(checksum != check){
        fprintf(stderr, "Did not retain total funds in accounts. Program failed\n");
        free(thread_args);
        return 1;
    }

    printf("Lock mode %d | Time elapsed: %lf sec | Checksum: %lld (OK) | Total inquiry sum: %lld\n",
           lock, time_elapsed, check, total_inquiries);

    free(thread_args);
    return 0;
}

void* bank_worker_0(void* args){
    Args* argsin = (Args*)args;
    int inq_transactions = ceil(argsin->transactions * (argsin->inquiries/100.0));
    int transfers = argsin->transactions - inq_transactions;
    long long int* accounts = argsin->accounts_1;
    int count = argsin->accounts_num;
    long long int sum = 0;
    //Randomly choose transaction type in each loop
    //Randomly choose account to perform transaction on 
    for (int i = 0; i < argsin->transactions; i++){
        int type;
        if (inq_transactions == 0) type = 1;
        else if (transfers == 0) type = 2;
        else type = rand() % 2;
        int target = rand() % count;
        if(type == 1){//transfer
            int recipient;
            do{
                recipient = rand() % count;
                
            }while(recipient == target);

            //based on lock type -> different lock/unlock func
            if (argsin->mode == COARSE_MUTEX) pthread_mutex_lock(&coarse_mutex);
            else pthread_rwlock_wrlock(&coarse_rwlock);

            if (accounts[target] > 0) {
                long long int to_move = rand() % accounts[target];
                accounts[recipient] += to_move;
                accounts[target] -= to_move;
            }
            if (argsin->mode == COARSE_MUTEX) pthread_mutex_unlock(&coarse_mutex);
            else pthread_rwlock_unlock(&coarse_rwlock);   

             transfers--;
        }
        else{//inquiry
            if (argsin->mode == COARSE_MUTEX) pthread_mutex_lock(&coarse_mutex);
            else pthread_rwlock_rdlock(&coarse_rwlock);

            //local sum of inquiries
            sum+= accounts[target];

            if (argsin->mode == COARSE_MUTEX) pthread_mutex_unlock(&coarse_mutex);
            else pthread_rwlock_unlock(&coarse_rwlock);   

            inq_transactions--;
        } 
    }

    //update inquries sum
    argsin->priv_sum = sum;
    return NULL;
}

void* bank_worker_1(void* args){
    Args* argsin = (Args*)args;

    int inq_transactions = ceil(argsin->transactions * (argsin->inquiries/100.0));
    int transfers = argsin->transactions - inq_transactions;

    Account* accounts = argsin->accounts_2;
    int count = argsin->accounts_num;
    long long int sum = 0;

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

            if(argsin->mode == FINE_MUTEX ){
                pthread_mutex_lock(&accounts[first].fine_mutex);
                pthread_mutex_lock(&accounts[second].fine_mutex);
            }else{
                pthread_rwlock_wrlock(&accounts[first].fine_rwlock);
                pthread_rwlock_wrlock(&accounts[second].fine_rwlock);
            }

            if (accounts[target].balance > 0) {
                long long int to_move = rand() % accounts[target].balance;
                accounts[recipient].balance += to_move;
                accounts[target].balance -= to_move;
            }

            if(argsin->mode == FINE_MUTEX){
                pthread_mutex_unlock(&accounts[second].fine_mutex);
                pthread_mutex_unlock(&accounts[first].fine_mutex);
            }else{
                pthread_rwlock_unlock(&accounts[first].fine_rwlock);
                pthread_rwlock_unlock(&accounts[second].fine_rwlock);

            }
            transfers--;
        }else{
            if(argsin->mode == FINE_MUTEX)pthread_mutex_lock(&accounts[target].fine_mutex);
            else pthread_rwlock_rdlock(&accounts[target].fine_rwlock);

            sum+=accounts[target].balance;

            if (argsin->mode == FINE_MUTEX) pthread_mutex_unlock(&accounts[target].fine_mutex);
            else pthread_rwlock_unlock(&accounts[target].fine_rwlock);           
            inq_transactions--;
        }
    }
    argsin->priv_sum = sum; 
    return NULL;
}
