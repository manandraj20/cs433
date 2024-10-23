import numpy as np
import random

def generate_test_case(n, filename_matrix, filename_vector):
    # Generate lower triangular matrix L with random float values (0 to 100)
    L = np.tril(np.random.randint(0, 100, (n, n)))

    # Ensure diagonal elements are non-zero
    for i in range(n):
        if L[i, i] == 0:
            L[i, i] = random.randint(1, 100)

    # Truncate L to six decimal places
    L = np.round(L, 6)

    # Generate random vector X of size n and truncate to six decimal places
    X = np.round(np.random.randint(0, 100, n), 6)

    # Calculate y = L * X
    y = np.dot(L, X)

    # Write the matrix L and y vector to filename_matrix
    with open(filename_matrix, 'w') as f:
        # Write the value of n
        f.write(f"{n}\n")

        # Write only the lower triangular part of the matrix L row-wise
        for i in range(n):
            # Only write up to the diagonal (i.e., elements in L[i][0] to L[i][i])
            f.write(" ".join(f"{L[i, j]:.6f}" for j in range(i+1)) + "\n")

        # Write the y vector without limiting precision
        f.write(" ".join(str(val) for val in y) + "\n")

    # Write the X vector to filename_vector
    with open(filename_vector, 'w') as f:
        f.write(" ".join(f"{val:.6f}" for val in X) + "\n")

# Example usage:
# generate_test_case(1000, "./Q2T1.txt", "./Q2A1.txt")
if __name__ == "__main__":
    # take input from user
    n = int(input("Enter the value of n: "))
    filename_matrix = input("Enter the filename for matrix L and y: ")
    filename_vector = input("Enter the filename for vector X: ")
    generate_test_case(n, filename_matrix, filename_vector)
