import random

def generate_test_case(n, file_name):
    # Generate a random lower triangular matrix L
    L = [[0.0] * n for _ in range(n)]
    for i in range(n):
        for j in range(i + 1):  # Ensure it's lower triangular
            L[i][j] = round(random.uniform(-10, 10), 2)  # Random values between -10 and 10

    # Generate a random vector x
    x = [round(random.uniform(-10, 10), 2) for _ in range(n)]

    # Compute vector y as L * x
    y = [0.0] * n
    for i in range(n):
        y[i] = sum(L[i][j] * x[j] for j in range(i + 1))

    # Write to the output file
    with open(file_name, 'w') as f:
        # Write the size of the matrix
        f.write(f"{n}\n")
        
        # Write the lower triangular matrix L
        for i in range(n):
            row = " ".join(str(L[i][j]) for j in range(i + 1))
            f.write(row + "\n")

        # Write the vector y
        f.write(" ".join(str(y_i) for y_i in y) + "\n")
        
        # Write the vector x
        # f.write(" ".join(str(x_i) for x_i in x) + "\n")

# Generate a test case with 100x100 matrix
generate_test_case(100, "test_case_100.txt")
