#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cuda.h>
#include <sys/time.h>

__global__ void initialize_1D(float *A, float *x, int n, int span)
{
    int i, j;
    int id = threadIdx.x + blockIdx.x * blockDim.x;
    for (j = 0; j < n; j++)
        for (i = span * id; i < span * (id + 1); i++)
        {

            A[j + n*i] = 1;
        }
    for (i = span * id; i < span * (id + 1); i++)
    {
        x[i] = 1;
    }
}

__global__ void initialize_2D(float **A, float *x, int n, int span)
{
    int i, j;
    int id_x = threadIdx.x + blockIdx.x * blockDim.x;
    int id_y = threadIdx.y + blockIdx.y * blockDim.y;

    for (i = span * id_x; i < span * (id_x + 1); i++)
    {
        x[i] = 1;
    }
    for(j = span * id_y; j < span * (id_y + 1); j++)
        for (i = span * id_x; i < span * (id_x + 1); i++)
        {
            A[j][i] = 1;
        }
}

__global__ void solve(float *A, float *x, float *y,  int n, int span)
{
    // solve for y = Ax
    int i, j;
    int id = threadIdx.x + blockIdx.x * blockDim.x;
    for (i = span * id; i < span * (id + 1); i++)
    {
        y[i] = 0;
        for (j = 0; j < n; j++)
        {
            y[i] += A[n*i + j] * x[j];
        }
    }
}

int main(int argc, char *argv[])
{
    float *A, *x, *y;
    int n;
    int nthreads;
    struct timeval tv0, tv2, tv1;
    struct timezone tz0, tz2, tz1;


    if (argc != 3)
    {
        printf("Need number of threads.\n");
        exit(1);
    }
    nthreads = atoi(argv[1]);
    assert((nthreads & (nthreads - 1)) == 0);

    n = atoi(argv[2]);
    assert((n & (n - 1)) == 0);
    int device = -1;
    cudaGetDevice(&device);
    cudaMallocManaged((void **)&A, n *n* sizeof(float));
    // cudaMemAdvise(A, n * sizeof(float *), cudaMemAdviseSetAccessedBy, device);
    // for (int i = 0; i < n; i++)
    // {
    //     cudaMallocManaged((void **)&A[i], n * sizeof(float));
    //     // cudaMemAdvise(A[i], n * sizeof(float),cudaMemAdviseSetAccessedBy, device);
    // }

    cudaMallocManaged((void **)&x, n * sizeof(float));
    // cudaMemAdvise(x, n * sizeof(float), cudaMemAdviseSetAccessedBy, device);

    gettimeofday(&tv0, &tz0);

    // dim3 grid_dims, block_dims;
    // grid_dims.x = n>>4;
    // block_dims.x = 16;
    //TODO:  make to sure handle the case when y is limited to 1024
    // grid_dims.y = n>>4;
    // block_dims.y = 16;



    nthreads = min(n, nthreads);
    if (nthreads < 32)
    {
        initialize_1D<<<1, nthreads>>>(A, x, n, n / nthreads);
    }
    else
    {
        initialize_1D<<<nthreads/8, 8>>>(A, x, n, n / nthreads);
        // initialize_2D<<<grid_dims, block_dims>>>(A, x, n, n / nthreads);
    }
    cudaDeviceSynchronize();
    
    cudaMallocManaged((void**)&y, sizeof(float)*n);
    cudaMemAdvise(x, sizeof(float)*n, cudaMemAdviseSetReadMostly, 0);
    cudaMemAdvise(A, n *n* sizeof(float),cudaMemAdviseSetReadMostly,0);
    // for (int i = 0; i < n; i++)
    // {
    //     cudaMemAdvise(A[i], n * sizeof(float),cudaMemAdviseSetReadMostly,0);
    // }
    gettimeofday(&tv1, &tz1);
    
    if(nthreads < 16)
    {
        solve<<<1, nthreads>>>(A, x, y, n, n / nthreads);
    }
    else
    {
        solve<<<nthreads / 8, 8>>>(A, x, y, n, n / nthreads);
    }
    cudaDeviceSynchronize();
    gettimeofday(&tv2, &tz2);

    printf("Random element: %lf, time: %ld microseconds\n", y[random() % n], (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec));
    // printf("Random element: %lf, time: %ld microseconds\n", A[random() % n], (tv2.tv_sec - tv0.tv_sec) * 1000000 + (tv2.tv_usec - tv0.tv_usec));

    // free the resources
    cudaFree(A);
    cudaFree(x);
    cudaFree(y);

    return 0;
}
