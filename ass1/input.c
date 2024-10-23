#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_test_case(int n, const char* filename_matrix, const char* filename_vector) {
    // Allocate memory for the matrix L and vectors X and y
    int** L = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        L[i] = (int*)malloc(n * sizeof(int));
    }

    int* X = (int*)malloc(n * sizeof(int));
    long long* y = (long long*)malloc(n * sizeof(long long));

    // Seed the random number generator
    srand(time(0));

    // Generate lower triangular matrix L with random integer values (0 to 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            L[i][j] = rand() % 100;
        }
        for (int j = i + 1; j < n; j++) {
            L[i][j] = 0;  // Upper part is zero
        }

        // Ensure diagonal elements are non-zero
        if (L[i][i] == 0) {
            L[i][i] = (rand() % 100) + 1;
        }
    }

    // Generate random vector X of size n
    for (int i = 0; i < n; i++) {
        X[i] = rand() % 100;
    }

    // Calculate y = L * X
    for (int i = 0; i < n; i++) {
        y[i] = 0;
        for (int j = 0; j <= i; j++) {
            y[i] += (long long)L[i][j] * X[j];
        }
    }

    // Write the matrix L and y vector to filename_matrix
    FILE* file_matrix = fopen(filename_matrix, "w");
    if (file_matrix == NULL) {
        printf("Error opening file for writing: %s\n", filename_matrix);
        return;
    }

    // Write the value of n
    fprintf(file_matrix, "%d\n", n);

    // Write the lower triangular part of matrix L
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            fprintf(file_matrix, "%d ", L[i][j]);
        }
        fprintf(file_matrix, "\n");
    }

    // Write the y vector
    for (int i = 0; i < n; i++) {
        fprintf(file_matrix, "%lld ", y[i]);
    }
    fprintf(file_matrix, "\n");
    fclose(file_matrix);

    // Write the X vector to filename_vector
    FILE* file_vector = fopen(filename_vector, "w");
    if (file_vector == NULL) {
        printf("Error opening file for writing: %s\n", filename_vector);
        return;
    }

    for (int i = 0; i < n; i++) {
        fprintf(file_vector, "%d ", X[i]);
    }
    fprintf(file_vector, "\n");
    fclose(file_vector);

    // Free allocated memory
    for (int i = 0; i < n; i++) {
        free(L[i]);
    }
    free(L);
    free(X);
    free(y);
}

int main() {
    int n;
    char filename_matrix[256], filename_vector[256];

    // Input from user
    printf("Enter the value of n: ");
    scanf("%d", &n);
    printf("Enter the filename for matrix L and y: ");
    scanf("%s", filename_matrix);
    printf("Enter the filename for vector X: ");
    scanf("%s", filename_vector);

    generate_test_case(n, filename_matrix, filename_vector);

    return 0;
}
