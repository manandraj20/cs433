#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_test_case(int n, const char* filename_matrix) {
    // Allocate memory for the distance matrix
    int** dist = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        dist[i] = (int*)malloc(n * sizeof(int));
    }

    // Seed the random number generator
    srand(time(0));

    // Generate random distance values (positive integers) for the upper triangular matrix
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            dist[i][j] = (rand() % 100) + 1; // Distances from 1 to 100
            dist[j][i] = dist[i][j];         // Symmetric for undirected graph
        }
        dist[i][i] = 0;  // Distance to self is 0
    }

    // Write the matrix in the specified format to the output file
    FILE* file_matrix = fopen(filename_matrix, "w");
    if (file_matrix == NULL) {
        printf("Error opening file for writing: %s\n", filename_matrix);
        return;
    }

    // Write the value of n (number of vertices)
    fprintf(file_matrix, "%d\n", n);

    // Write the distances as per the required format
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            fprintf(file_matrix, "%d ", dist[i][j]);
        }
        fprintf(file_matrix, "\n");
    }

    fclose(file_matrix);

    // Free dynamically allocated memory
    for (int i = 0; i < n; i++) {
        free(dist[i]);
    }
    free(dist);
}

int main() {
    int n;
    char filename_matrix[256];

    // Input from the user
    printf("Enter the number of vertices (n): ");
    scanf("%d", &n);
    printf("Enter the filename for the distance matrix: ");
    scanf("%s", filename_matrix);

    // Generate test case and write to file
    generate_test_case(n, filename_matrix);

    return 0;
}
