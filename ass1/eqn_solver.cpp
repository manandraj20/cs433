#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <omp.h>

using namespace std;

// Function to initialize L randomly, create random x, and compute y = L * x
void InitializeInput(vector<vector<double>> &L, vector<double> &y, int n) {
    srand(time(0)); // Seed for random number generation

    L.resize(n, vector<double>(n, 0.0)); // Initialize L with zeros
    vector<double> x(n);  // Randomly initialized x, not accessible outside

    // Fill L as a lower triangular matrix with random values and initialize random x
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            L[i][j] = (rand() % 100) / 10.0; // Random values between 0.0 and 10.0
        }
        x[i] = (rand() % 100) / 10.0; // Random values for x between 0.0 and 10.0
    }

    // Compute y = L * x
    y.resize(n, 0.0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            y[i] += L[i][j] * x[j];
        }
    }
}



// Function to read input from a file
void ReadInput(const string &filename, vector<vector<double>> &L, vector<double> &y, int &n) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error opening input file." << endl;
        exit(EXIT_FAILURE);
    }

    infile >> n;
    L.resize(n, vector<double>(n, 0.0));  // Initialize an n x n matrix with zeros
    y.resize(n);

    // Read matrix L
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {  // Only read up to the diagonal for lower triangular matrix
            infile >> L[i][j];
        }
    }

    // Read vector y
    for (int i = 0; i < n; i++) {
        infile >> y[i];
    }

    infile.close();
}

// Function to solve the lower triangular system Lx = y without parallelization
void SolveWithoutParallelization(const vector<vector<double>> &L, const vector<double> &y, vector<double> &x, int n) {
 
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < i; j++) {
            sum += L[i][j] * x[j];
        }
        x[i] = (y[i] - sum) / L[i][i];  // Forward substitution
    }
}

// Function to solve the lower triangular system Lx = y
void SolveLowerTriangular(const vector<vector<double>> &L, const vector<double> &y, vector<double> &x, int n, int nthreads, double *localsum) {

    int tid;
    int i, j;
    # pragma omp parallel num_threads(nthreads) private(i,j,tid)
    for (i = 0; i < n; i++) {
        tid = omp_get_thread_num();
        
        # pragma omp for schedule(static, 4) 
        // #pragma omp parallel for num_threads (nthreads) private(tid) schedule(static, 8)
        for (j = 0; j < i; j++) {
            // #pragma omp atomic
            // sum += L[i][j] * x[j];
           
            localsum[10*tid]+=L[i][j] * x[j];
        }    
        if(tid==0)
        { 
            double sum = 0.0;
            for(int k=0;k<nthreads*10;k+=10)
        {
            sum+=localsum[k];
            localsum[k] = 0.0;
        }
        x[i] = (y[i] - sum) / L[i][i]; 
        }
        #pragma omp barrier

    }
}

// Function to write the solution vector x to an output file
void WriteOutput(const string &filename, const vector<double> &x) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error opening output file." << endl;
        exit(EXIT_FAILURE);
    }

    for (double xi : x) {
        outfile << xi << " ";
    }
    outfile << endl;
    outfile.close();
}

// Main function
int main(int argc, char *argv[]) {
    struct timeval tv0, tv1;
	struct timezone tz0, tz1;

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file> <num_threads>" << endl;
        return EXIT_FAILURE;
    }

    string input_file = argv[1];
    string output_file = argv[2];
    int num_threads = stoi(argv[3]);  // This is not used in the non-parallelized version

    int n;  // Size of the matrix
    vector<vector<double>> L;  // Lower triangular matrix
    vector<double> y;  // Right-hand side vector
    vector<double> x;  // Solution vector

    // Initialize input for testing
    n = 20000;  
    // InitializeInput(L, y, n);  // Replace 3 with the desired size n

    // Alternatively, read input from a file
    // ReadInput(input_file, L, y, n);
    x.resize(n);
    // set y and L to all 1s
    L.resize(n, vector<double>(n, 0.0));  // Initialize an n x n matrix with zeros
    y.resize(n);
    for (int i = 0; i < n; i++) {
        y[i] = i + 1.0;
        for (int j = 0; j <= i; j++) {
            L[i][j] = j+2;
        }
    }
    double localsum[num_threads*10];
    for(auto &ele: localsum){
        ele = 0.0;
    }
    gettimeofday(&tv0, &tz0);

    // Solve the system of equations without parallelization
    // SolveWithoutParallelization(L, y, x, L.size());
    SolveLowerTriangular(L, y, x, n, num_threads, localsum);
    gettimeofday(&tv1, &tz1);

    printf("Time: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));

    // Write output to file
    WriteOutput(output_file, x);

    return 0;
}
