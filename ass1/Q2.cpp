#include <iostream>
#include <fstream>
#include <vector>
#include <limits.h>
#include <sys/time.h>
#include <omp.h>
#include <assert.h>
#include <iomanip> 

using namespace std;

int main(int argc, char *argv[]) {
    if(argc != 4) {
        cout << "Need 3 arguments.\n";
        exit(1);
    }
    int nThreads = atoi(argv[3]);
    ifstream fin;
    fin.open(argv[1]);
    ofstream fout;
    fout.open(argv[2]);
    assert(fin);
    assert(fout);

    struct timeval tv0, tv1;
    struct timezone tz0, tz1;
    // int n = 30000;
    int n;
    fin >> n;
    vector<vector<long double>> L(n, vector<long double>(n, 0));
    vector<long double> x(n, 0);
    long double y[n] = {0};
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < i + 1; j++) {
            fin >> L[i][j];
	    }
    }
    for(int i = 0; i < n; i++){
        fin >> y[i];
    }

    gettimeofday(&tv0, &tz0);
    
vector<long double> localsum(nThreads * 12, 0.0);
int number_of_doubles_per_cache_line = 12;
int i, j, tid;
#pragma omp parallel num_threads(nThreads) private(i, j, tid)
    for (i = 0; i < n; i++) {
        tid = omp_get_thread_num();
#pragma omp for 
        for (j = 0; j < i; j++) {
            localsum[number_of_doubles_per_cache_line * tid] += L[i][j] * x[j];  // Compute local sum
        }
        
        if (tid == 0)
        {
            long double sum = 0.0;
            for (int k = 0; k < nThreads * number_of_doubles_per_cache_line; k += number_of_doubles_per_cache_line) {
                sum += localsum[k];
                localsum[k] = 0.0;
            }
            x[i] = (y[i] - sum) / L[i][i];  // Update x[i] after synchronization
        }
        #pragma omp barrier  // Ensure all threads synchronize before moving to the next row
    }

    gettimeofday(&tv1, &tz1);

    for(int i = 0; i < n; i++) 
        fout << x[i] << " ";
    fout << endl;
    cout << "Size of matrix: " << n << "*" << n << "\n";
    cout << "Number of threads: " << nThreads << "\n";
    cout << "Time: " << (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec) << " microseconds\n";
    fin.close();
    fout.close();
    return 0;
}