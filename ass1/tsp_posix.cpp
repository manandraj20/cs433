#include <omp.h>
#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>
#include <ctime>
#include <sys/time.h>

using namespace std;

// Function to compute the cost of a given tour
int compute_cost(const vector<int>& tour, const vector<vector<int>>& weights) {
    int cost = 0;
    for (int i = 0; i < tour.size() - 1; i++) {
        cost += weights[tour[i]][tour[i + 1]];
    }
    cost += weights[tour[tour.size() - 1]][tour[0]]; // Return to start
    return cost;
}

// Helper function to calculate factorial
int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

// Helper function to advance to the k-th lexicographical permutation
template <typename It>
void advance_to_permutation(It begin, It end, int k) {
    int n = distance(begin, end);
    vector<int> fact(n);
    for (int i = 0; i < n; i++) {
        fact[i] = factorial(n - 1 - i);
    }

    for (int i = 0; i < n; i++) {
        int idx = k / fact[i];
        k %= fact[i];
        rotate(begin + i, begin + i + idx, end);
        sort(begin + i + 1, end);  // Ensure the rest remains sorted
    }
}

// Parallel TSP function using OpenMP
void tsp_parallel(const vector<vector<int>>& weights, int num_threads, vector<int>& best_path, int& min_cost) {
    int n = weights.size();
    vector<int> cities(n);
    for (int i = 0; i < n; i++) cities[i] = i;

    min_cost = INT_MAX;

    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num(); // Thread ID
        int total_threads = omp_get_num_threads(); // Total number of threads

        vector<int> perm = cities; // Initial permutation {A, B, C, D, ...}
        int total_permutations = factorial(n - 1); // Total permutations of n-1 cities

        // Calculate range for this thread
        int per_thread_count = total_permutations / total_threads;
        int start_index = tid * per_thread_count;
        int end_index = (tid == total_threads - 1) ? total_permutations : start_index + per_thread_count;

        // Generate permutations in the assigned range
        advance_to_permutation(perm.begin() + 1, perm.end(), start_index); // Move to the thread's starting permutation

        for (int perm_count = start_index; perm_count < end_index; perm_count++) {
            int cost = compute_cost(perm, weights);

            #pragma omp critical
            {
                if (cost < min_cost) {
                    min_cost = cost;
                    best_path = perm;
                }
            }

            next_permutation(perm.begin() + 1, perm.end());
        }
    }
}

int main(int argc, char* argv[]) {
      struct timeval tv0, tv1;
	struct timezone tz0, tz1;
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <input_file> <output_file> <num_threads>" << endl;
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];
    int num_threads = stoi(argv[3]);

    // Read the input file and construct the weights matrix
    vector<vector<int>> weights(1000, vector<int> (1000, 1.0));
    // Populate weights based on the input file (omitted for brevity)

    vector<int> best_path;
    int min_cost;
    gettimeofday(&tv0, &tz0);
    tsp_parallel(weights, num_threads, best_path, min_cost);
    gettimeofday(&tv1, &tz1);

    printf("Time: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));

    cout<<min_cost<<endl;
    // Write output to the output file
    // Write best_path and min_cost to the output file (omitted for brevity)

    return 0;
}
