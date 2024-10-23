#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <omp.h>
#include <sys/time.h>

// Define the number of nodes in the graph (0-based)
const int n = 10; // 0 to 3 for 4 nodes
const int MAX = 1000000;  // A large value to represent infinity


int dist[30][30];

int main(int argc, char *argv[]) {
    struct timeval tv0, tv1;
	struct timezone tz0, tz1;
	int nthreads;
    int i, j, k, l, mask;
    int **dp;  // Dynamic programming table
    int **parent;
    int **done;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            dist[i][j] = 1;
        }
    }
    dist[0][1] = dist[1][0] = 10;
    for (i = 0; i < n; i++) {
        dist[i][i] = MAX;
    }

    if (argc != 2) {
		printf ("Need number of threads.\n");
		exit(1);
	}
	nthreads = atoi(argv[1]);

    done = (int **)malloc(n * sizeof(int *));
    // Dynamically allocate memory for the DP table
    dp = (int **)malloc(n * sizeof(int *));
    assert(dp != NULL);  // Ensure memory allocation was successful
    parent = (int **)malloc(n * sizeof(int *));
    assert(parent != NULL);  // Ensure memory allocation was successful

    long long int size = (1 << n);
    for (i = 0; i < n; i++) {
        dp[i] = (int *)malloc(size * sizeof(int));
    }
    for (i = 0; i < n; i++) {
        done[i] = (int *)malloc(size * sizeof(int));
    }
    

    // Initialize the DP table with large values (infinity)
    for (i = 0; i < n; i++) {
        for (j = 0; j < (1 << n); j++) {
            dp[i][j] = MAX;
            done[i][j] = 0;
        }
    }

    

    for(i=0;i<n;i++){
        parent[i] = (int *)malloc(size * sizeof(int));
    }
   
    for (i = 0; i < n; i++) {
        for (j = 0; j < (1 << n); j++) {
            parent[i][j] = -1;
        }
    }
    // Initialize base case for DP: distance from node 0 to node i
    for (i = 1; i < n; i++) { // start from 1 since we want to compute paths starting from node 0
        dp[i][(1 << i) | 1] = dist[0][i];
        parent[i][(1 << i) | 1] = 0;
        done[i][(1 << i) | 1] = 1;
        
    }
    int *setBits = (int *)malloc(n * sizeof(int));
    gettimeofday(&tv0, &tz0);
    int tid;
    int total_set_bits;
    // Bottom-up DP to calculate minimum cost paths
    #pragma omp parallel for num_threads (nthreads) schedule(static) private(i,j, k, l, total_set_bits)
    for (long long int mask = 0; mask < (1 << n); mask++) {
        
            total_set_bits=0;
            for (i = 1; i < n; i++) {
                if(mask & (1 << i)){
                    setBits[total_set_bits] = i;
                    total_set_bits++;
                }
            }
        

        for (k = 0; k < total_set_bits; k++) {
                for (l = 0; l < total_set_bits; l++) {
                    i = setBits[k];
                    j = setBits[l];
                    if (j != i) {  
                        int reduced_mask = mask & ~(1 << i);
                        while(!done[j][reduced_mask]);
                        int new_cost = dp[j][reduced_mask] + dist[j][i];
                        if(new_cost < dp[i][mask]){
                            dp[i][mask] = new_cost;
                            parent[i][mask] = j;
                        }
                    }
                    
                }
                done[i][mask] = 1;
                printf("i: %d, mask: %lld, done: %d, thread: %d\n", i, mask, done[i][mask], omp_get_thread_num());
            }
        }

    gettimeofday(&tv1, &tz1);

    // Calculate the final result: minimum cost of the most efficient tour
    int ans = MAX;
    int optimal_node;
    for (i = 1; i < n; i++) {  // Start from node 1 to n-1
        if (dp[i][(1 << n) - 1] + dist[i][0] < ans) {
            optimal_node = i;
            ans = dp[i][(1 << n) - 1] + dist[i][0];
        }
    }

    
    printf("Time: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));
    
    int mask_trace = (1 << n) - 1;  // All nodes visited
    int node = optimal_node;        // The node that gives the minimum cost tour
    printf("The optimal path is: 0 ");  // Start from node 0

    while (node != -1) {
        printf("%d ", node);
        int next_node = parent[node][mask_trace];
        mask_trace &= ~(1 << node);  // Remove the current node from the mask
        node = next_node;
    }

    printf("0\n");  // Return to the starting node


    // Print the result
    printf("The cost of the most efficient tour = %d\n", ans);
    // Free dynamically allocated memory
    for (i = 0; i < n; i++) {
        free(dp[i]);
    }
    free(dp);

    return 0;
}
