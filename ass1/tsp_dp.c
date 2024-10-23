#include <stdlib.h>
#include<stdio.h>
#include <sys/time.h>
#include <omp.h>
#include <assert.h>
#define ll long long
#define l long 

int main(int argc, char *argv[]) {
    ll int i, j, n, tid;
    struct timeval tv0, tv1;
    struct timezone tz0, tz1;
    double **L, *y, *x;  // Declare L as a pointer to pointers for a dynamic 2D array
    double *localsum;
    ll int num_threads;

    // Check if correct number of arguments are passed
    if (argc != 4) {
        printf("Usage: %s <input_filename> <output_filename> <num_threads>\n", argv[0]);
        return -1;
    }

    num_threads = atoi(argv[3]); // Number of threads from command line

    // Open input file
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Error: Cannot open input file %s\n", argv[1]);
        return -1;
    }

    // Read n from input file
    fscanf(input_file, "%lld", &n);

    // Dynamically allocate memory for the 2D array L
    L = (double **)malloc(n * sizeof(double *));  // Allocate memory for n pointers (rows)
    for (i = 0; i < n; i++) {
        L[i] = (double *)malloc(n * sizeof(double));  // Allocate memory for each row (columns)
    }
    
    // Dynamically allocate memory for y, x, and localsum
    y = (double *)malloc(n * sizeof(double));
    x = (double *)malloc(n * sizeof(double));
    int number_of_doubles_per_cache_line = 12;
    localsum = (double *)malloc(num_threads * number_of_doubles_per_cache_line * sizeof(double));

    assert(L != NULL && y != NULL && x != NULL && localsum != NULL);

    // Read matrix L from the input file
    for (i = 0; i < n; i++) {
        for (j = 0; j <= i; j++) {
            fscanf(input_file, "%lf", &L[i][j]);  // Read lower triangular elements
        }
    }

    // Read vector y from the input file
    for (i = 0; i < n; i++) {
        fscanf(input_file, "%lf", &y[i]);
    }

    // Close input file after reading
    fclose(input_file);

    // Initialize localsum
    for (i = 0; i < num_threads * number_of_doubles_per_cache_line; i++) {
        localsum[i] = 0.0;  // Initialize localsum
    }

    gettimeofday(&tv0, &tz0);  // Start time measurement


    for (i = 0; i < n; i++) {

double sum = 0.0;
        for (j = 0; j < i; j++) {
            sum += L[i][j] * x[j];  // Compute local sum
        }
     
            x[i] = (y[i] - sum) / L[i][i];  // Update x[i] after synchronization
    
    }

    gettimeofday(&tv1, &tz1);  // End time measurement

    // Print execution time
    printf("Time: %ld microseconds\n", (tv1.tv_sec - tv0.tv_sec) * 1000000 + (tv1.tv_usec - tv0.tv_usec));

    // Open output file
    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        printf("Error: Cannot open output file %s\n", argv[2]);
        return -1;
    }

    // Write the full x vector to the output file
    for (i = 0; i < n; i++) {
        fprintf(output_file, "%lf ", x[i]);
    }
    fprintf(output_file, "\n");

    // Close output file
    fclose(output_file);

    // Free allocated memory
    for (i = 0; i < n; i++) {
        free(L[i]);  // Free memory for each row
    }
    free(L);  // Free memory for the array of pointers
    free(y);
    free(x);
    free(localsum);

    return 0;
}
