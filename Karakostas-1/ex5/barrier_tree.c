#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
    struct Node *parent;   // Next node up the tree 
    int expected;          // How many arrivals this node waits for 
    int count;             // Current arrivals left
    volatile bool flag;    // Sense flag to release threads waiting at this node
} Node;

static Node *leaves;       
static Node *all_nodes;    
static long long iterations = 0;

void tree_init(int num_threads);
void node_wait(Node *node, bool sense);
void *worker(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <iterations>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    iterations = atoll(argv[2]);

    tree_init(num_threads);

    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    int *tids = malloc((size_t)num_threads * sizeof(int));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        tids[i] = i;
        pthread_create(&threads[i], NULL, worker, &tids[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Combining Tree Barrier | Threads: %d | Iterations: %lld | Time: %.6f sec\n",
           num_threads, iterations, elapsed);

    free(all_nodes);
    free(tids);
    free(threads);
    return 0;
}

void tree_init(int num_threads) {
    // Find the next power of 2 >= num_threads for the leaf level
    int leaf_count = 1;
    while (leaf_count < num_threads) {
        leaf_count *= 2;
    }

    int total_nodes = 2 * leaf_count - 1;
    all_nodes = calloc((size_t)total_nodes, sizeof(Node));
    leaves = &all_nodes[leaf_count - 1]; // Leaves sit in the second half of the array

    //  Connect every node to its parent 
    for (int i = 1; i < total_nodes; i++) {
        all_nodes[i].parent = &all_nodes[(i - 1) / 2];
    }

    //  Mark active leaves and count how many active children each parent has
    for (int t = 0; t < num_threads; t++) {
        leaves[t].expected = 1;
    }
    for (int i = total_nodes - 1; i > 0; i--) {
        if (all_nodes[i].expected > 0) {
            all_nodes[i].parent->expected++;
        }
    }

    for (int i = 0; i < total_nodes; i++) {
        all_nodes[i].count = all_nodes[i].expected;
    }
}

void node_wait(Node *node, bool sense) {
    if (__atomic_sub_fetch(&node->count, 1, __ATOMIC_ACQ_REL) == 0) {
        if (node->parent != NULL) {
            node_wait(node->parent, sense);
        }
        node->count = node->expected;
        node->flag = sense;
    } else {
        while (node->flag != sense) {
            // busy-wait
        }
    }
}

void *worker(void *arg) {
    int tid = *(int *)arg;
    bool sense = false;

    for (long long i = 0; i < iterations; i++) {
        sense = !sense;                  
        node_wait(&leaves[tid], sense);
    }
    return NULL;
}
