#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <omp.h>
#include <sys/time.h>

// Define the number of nodes in the graph (0-based)
const int MAX = 1000000; // A large value to represent infinity

int **dist; // Distance matrix to store edge weights
int n;      // Number of vertices in the graph

int main(int argc, char *argv[])
{
    struct timeval tv0, tv1;
    struct timezone tz0, tz1;
    int nthreads;
    long long int i, j, k, l, mask;
    int **dp; // Dynamic programming table
    int **parent;
    int **done; // Done array for synchronization

    if (argc != 4)
    {
        printf("Usage: %s <input_file> <output_file> <number_of_threads>\n", argv[0]);
        exit(1);
    }

    // Open input and output files
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL)
    {
        perror("Error opening output file");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    nthreads = atoi(argv[3]);

    // Read the number of vertices (n)
    fscanf(input_file, "%d", &n);

    // Dynamically allocate memory for the distance matrix (dist)
    dist = (int **)malloc(n * sizeof(int *));
    assert(dist != NULL); // Ensure memory allocation was successful
    for (i = 0; i < n; i++)
    {
        dist[i] = (int *)malloc(n * sizeof(int));
    }

    // Initialize the distance matrix to MAX (infinity)
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            dist[i][j] = MAX;
        }
    }

    // Read the distance matrix from the input file
    for (i = 0; i < n - 1; i++)
    {
        for (j = i + 1; j < n; j++)
        {
            fscanf(input_file, "%d", &dist[i][j]);
            dist[j][i] = dist[i][j]; // Since it's an undirected graph
        }
    }

    fclose(input_file); // Close the input file


    long long int size = (1 << n);
    // Dynamically allocate memory for the DP table and done array
    dp = (int **)malloc(size * sizeof(int *));
    done = (int **)malloc(size * sizeof(int *));
    assert(dp != NULL && done != NULL); // Ensure memory allocation was successful
    parent = (int **)malloc(size * sizeof(int *));
    assert(parent != NULL); // Ensure memory allocation was successful

    
    for (i = 0; i < size; i++)
    {
        dp[i] = (int *)malloc(n * sizeof(int));
        parent[i] = (int *)malloc(n * sizeof(int));
        done[i] = (int *)malloc(n * sizeof(int)); // Allocate memory for done array
    }

    // Initialize the DP table, parent, and done arrays
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < n; j++)
        {
            dp[i][j] = MAX;
            parent[i][j] = -1; // Initialize parent array with -1
            done[i][j] = 0;    // Mark all entries in the done array as 0 initially
        }
    }

    // Initialize base case for DP: distance from node 0 to node i
    for (i = 1; i < n; i++)
    { // Start from 1 since we want to compute paths starting from node 0
        long long int mask = (1 << i) | 1;
        dp[mask][i] = dist[0][i];
        parent[mask][i] = 0;
        done[mask][i] = 1; // Mark the base case as done
    }

    gettimeofday(&tv0, &tz0);
    int tid;
    int total_set_bits;

// Bottom-up DP to calculate minimum cost paths
#pragma omp parallel for num_threads(nthreads) schedule(static, 1) private(mask, i, j, tid, k, l, total_set_bits)
    for (mask = 1; mask < (1 << n); mask += 2)
    {
        int *setBits = (int *)malloc(n * sizeof(int));
        total_set_bits = 0;
        for (i = 1; i < n; i++)
        {
            if (mask & (1 << i))
            {
                setBits[total_set_bits] = i;
                total_set_bits++;
            }
        }
        if(total_set_bits<2){
            continue;
        }

        for (k = total_set_bits-1; k >=0; k--)
        {
            for (l = 0; l < total_set_bits; l++)
            {
                i = setBits[k];
                j = setBits[l];
                if (j != i)
                {
                    int reduced_mask = mask & ~(1 << i);
                    while (!done[reduced_mask][j]);

                    int new_cost = dp[reduced_mask][j] + dist[j][i];
                    if (new_cost < dp[mask][i])
                    {
                        dp[mask][i] = new_cost;
                        parent[mask][i] = j;
                    }
                }
            }
            done[mask][i] = 1; // Mark the current mask for all setBits[i] as done
        }
    }

    gettimeofday(&tv1, &tz1);

    // Calculate the final result: minimum cost of the most efficient tour
    int ans = MAX;
    int optimal_node;
    for (i = 1; i < n; i++)
    { // Start from node 1 to n-1
    long long mask = (1 << n) - 1;
        if (dp[mask][i] + dist[i][0] < ans)
        {
            optimal_node = i;
            ans = dp[mask][i] + dist[i][0];
        }
    }

    // Write time taken and the optimal path to the output file
    printf("Time: %ld microseconds\n", (tv1.tv_sec - tv0.tv_sec) * 1000000 + (tv1.tv_usec - tv0.tv_usec));

    int mask_trace = (1 << n) - 1; // All nodes visited
    int node = optimal_node;       // The node that gives the minimum cost tour
    fprintf(output_file, "1 ");    // Start from node 0

    while (node != -1)
    {
        fprintf(output_file, "%d ", node+1);
        int next_node = parent[mask_trace][node];
        mask_trace &= ~(1 << node); // Remove the current node from the mask
        node = next_node;
    }

    // Write the final cost of the most efficient tour
    fprintf(output_file, "\n%d", ans);

    fclose(output_file); // Close the output file

    // Free dynamically allocated memory
    for (i = 0; i < n; i++)
    {
        free(dp[i]);
        free(parent[i]);
        free(done[i]); // Free memory allocated for done array
        free(dist[i]);
    }
    free(dp);
    free(parent);
    free(done);
    free(dist);
    // free(setBits);

    return 0;
}
