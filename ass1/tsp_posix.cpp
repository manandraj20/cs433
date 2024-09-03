#include <omp.h>
#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>

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

void tsp_parallel(const vector<vector<int>>& weights, int num_threads, vector<int>& best_path, int& min_cost) {
    int n = weights.size();
    vector<int> cities(n);
    for (int i = 0; i < n; i++) cities[i] = i;

    min_cost = INT_MAX;
    vector<int> local_best_path;

    #pragma omp parallel for num_threads(num_threads) private(local_best_path) shared(min_cost)
    for (int i = 1; i < n; i++) {
        vector<int> perm = cities;
        swap(perm[1], perm[i]); // Fix the first city and permute the rest

        do {
            int cost = compute_cost(perm, weights);
            #pragma omp critical
            {
                if (cost < min_cost) {
                    min_cost = cost;
                    local_best_path = perm;
                }
            }
        } while (next_permutation(perm.begin() + 1, perm.end()));
    }

    best_path = local_best_path;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <input_file> <output_file> <num_threads>" << endl;
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];
    int num_threads = stoi(argv[3]);

    // Read the input file and construct the weights matrix
    vector<vector<int>> weights;
    // Populate weights based on the input file (omitted for brevity)

    vector<int> best_path;
    int min_cost;
    tsp_parallel(weights, num_threads, best_path, min_cost);

    // Write output to the output file
    // Write best_path and min_cost to the output file (omitted for brevity)

    return 0;
}
