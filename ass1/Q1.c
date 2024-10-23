#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <omp.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Need 3 arguments.\n");
        exit(1);
    }
    
    int nThreads = atoi(argv[3]);
    FILE *fin = fopen(argv[1], "r");
    FILE *fout = fopen(argv[2], "w");
    assert(fin);
    assert(fout);

    struct timeval tv0, tv1;
    struct timezone tz0, tz1;
    int n;
    fscanf(fin, "%d", &n);
    int **d = (int**)malloc(n * sizeof(int*));
    for(int i = 0; i < n; i++) {
        d[i] = (int*)malloc(n * sizeof(int));
    }

    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            fscanf(fin, "%d", &d[i][j]);
            d[j][i] = d[i][j];
        }
    }

    int m = 1 << n;
    int *mask = (int*)malloc((n + 1) * sizeof(int));
    int **dp = (int**)malloc(m * sizeof(int*));
    int **trans = (int**)malloc(m * sizeof(int*));
    for(int i = 0; i < m; i++) {
        dp[i] = (int*)malloc(n * sizeof(int));
        trans[i] = (int*)malloc(n * sizeof(int));
        for(int j = 0; j < n; j++) {
            dp[i][j] = INT_MAX;
        }
    }

    gettimeofday(&tv0, &tz0);

    // Precompute the mask sets
    for(int i = 1; i < m; i += 2) {
        int size = __builtin_popcount(i);
        mask[size] = i;
    }

    dp[1][0] = 0;

    #pragma omp parallel num_threads(nThreads)
    {
        for(int size = 2; size <= n; size++) {
            #pragma omp for
            for(int i = 0; i < mask[size]; i++) {
                int subset = mask[size];
                dp[subset][0] = INT_MAX;
                for(int i1 = 1; i1 < n; i1++) {
                    int val = dp[subset][i1], pos = -1;
                    for(int i2 = 0; i2 < n; i2++) {
                        int p1 = 1 << i1, p2 = 1 << i2;
                        if((p1 & subset) == 0 || (p2 & subset) == 0 || i1 == i2) continue;
                        if(dp[subset ^ p1][i2] == INT_MAX) continue;
                        if(val > dp[subset ^ p1][i2] + d[i1][i2]) {
                            val = dp[subset ^ p1][i2] + d[i1][i2];
                            pos = i2;
                        }
                    }
                    dp[subset][i1] = val;
                    trans[subset][i1] = pos;
                }
            }
        }
    }

    int ans = INT_MAX, st = -1;

    // Compute the final answer
    for(int i = 1; i < n; i++) {
        if(dp[m - 1][i] == INT_MAX) continue;
        if(dp[m - 1][i] + d[i][0] < ans) {
            ans = dp[m - 1][i] + d[i][0];
            st = i;
        }
    }

    int subset = m - 1;
    int *path = (int*)malloc(n * sizeof(int));
    int pathIdx = 0;
    path[pathIdx++] = 1;
    
    // Rebuild the path
    while(st != 0) {
        path[pathIdx++] = st + 1;
        int nxt = trans[subset][st];
        subset ^= 1 << st;
        st = nxt;
    }
    path[pathIdx++] = 1;

    gettimeofday(&tv1, &tz1);

    // Output the result
    for(int i = 0; i < pathIdx; i++) {
        fprintf(fout, "%d ", path[i]);
    }
    fprintf(fout, "\n%d\n", ans);
    
    printf("Number of vertices: %d\n", n);
    printf("Number of threads: %d\n", nThreads);
    printf("Time: %ld microseconds\n", (tv1.tv_sec - tv0.tv_sec) * 1000000 + (tv1.tv_usec - tv0.tv_usec));

    fclose(fin);
    fclose(fout);
    
    // Free the memory
    for(int i = 0; i < n; i++) {
        free(d[i]);
    }
    free(d);
    free(dp);
    free(trans);
    free(mask);
    free(path);
    
    return 0;
}
