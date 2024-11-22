#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cuda.h>
#include <sys/time.h>
#include <math.h>
#include <cooperative_groups.h>
namespace cg = cooperative_groups;

#define ROWS_A ((1 << 10) + 2)
#define COLS_A ((1 << 10) + 2)

#define THREADS_PER_BLOCK_X 16
#define THREADS_PER_BLOCK_Y 16

#define SPAN_PER_THREAD_X 8
#define SPAN_PER_THREAD_Y 8
#define BLOCKS_X ((COLS_A - 2) / (THREADS_PER_BLOCK_X * SPAN_PER_THREAD_X))
#define BLOCKS_Y ((ROWS_A - 2) / (THREADS_PER_BLOCK_Y * SPAN_PER_THREAD_Y))

#define TOL 1e-6
#define ITER_LIMIT 1000

__device__ int count = 0;
__device__ volatile int barrier_flag = 0;

__managed__ float global_diff;

__global__ void init_kernel(float *A)
{
    int col = (threadIdx.x + blockIdx.x * blockDim.x) * SPAN_PER_THREAD_X + 1;
    int row = (threadIdx.y + blockIdx.y * blockDim.y) * SPAN_PER_THREAD_Y + 1;

    for (int i = row; i < row + SPAN_PER_THREAD_Y; i++)
    {
        for (int j = col; j < col + SPAN_PER_THREAD_X; j++)
        {
            A[j + COLS_A * i] = (float)(row * col) / (ROWS_A * COLS_A);
        }
    }
}

__global__ void pad_cols_kernel(float *A)
{
    int id = threadIdx.x + blockIdx.x * blockDim.x;
    A[COLS_A * id] = 0.0;
    A[COLS_A * id + COLS_A - 1] = 0.0;
}

__global__ void pad_rows_kernel(float *A)
{
    int id = threadIdx.x + blockIdx.x * blockDim.x;
    A[id] = 0.0;
    A[COLS_A * (ROWS_A - 1) + id] = 0.0;
}

__global__ void grid_solver_kernel(float *A)
{
    int i, j, done = 0, iters = 0;
    float temp, local_diff;
    int local_sense = 0;
    int last_count;
    int col = (threadIdx.x + blockIdx.x * blockDim.x) * SPAN_PER_THREAD_X + 1;
    int row = (threadIdx.y + blockIdx.y * blockDim.y) * SPAN_PER_THREAD_Y + 1;

    while (!done)
    {
        local_diff = 0.0f;
        if (!blockIdx.x && !blockIdx.y)
        {
            global_diff = 0.0f;
        }
        // cg::grid_group grid = cg::this_grid();
        // grid.sync();
        local_sense = (local_sense ? 0 : 1);
        __syncthreads();
        if (threadIdx.x == 0)
        {
            last_count = atomicAdd(&count, 1);
            if (last_count == (BLOCKS_X - 1))
            {
                count = 0;
                barrier_flag = local_sense;
            }
        }
        while (barrier_flag != local_sense)
            ;

        for (i = row; i < row + SPAN_PER_THREAD_Y; i++)
        {
            for (j = col; j < col + SPAN_PER_THREAD_X; j++)
            {
                temp = A[j + COLS_A * i];
                A[j + COLS_A * i] = 0.2f * (A[j + COLS_A * i] + A[j + 1 + COLS_A * i] + A[j - 1 + COLS_A * i] + A[j + COLS_A * (i + 1)] + A[j + COLS_A * (i - 1)]);
                local_diff += fabsf(A[j + COLS_A * i] - temp);
            }
        }

        atomicAdd(&global_diff, local_diff);
        // cg::grid_group grid = cg::this_grid();
        // grid.sync();

        local_sense = (local_sense ? 0 : 1);
        __syncthreads();
        if (threadIdx.x == 0)
        {
            last_count = atomicAdd(&count, 1);
            if (last_count == (BLOCKS_X - 1))
            {
                count = 0;
                barrier_flag = local_sense;
            }
        }
        while (barrier_flag != local_sense)
            ;

        iters++;
        if (global_diff / (ROWS_A * COLS_A) < TOL || iters >= ITER_LIMIT)
        {
            done = 1;
        }
        // cg::grid_group grid = cg::this_grid();
        // grid.sync();

        local_sense = (local_sense ? 0 : 1);
        __syncthreads();
        if (threadIdx.x == 0)
        {
            last_count = atomicAdd(&count, 1);
            if (last_count == (BLOCKS_X - 1))
            {
                count = 0;
                barrier_flag = local_sense;
            }
        }
        while (barrier_flag != local_sense)
            ;
        if(!blockIdx.x && !blockIdx.y)
        {
            printf("Iteration: %d, Error: %f\n", iters, global_diff / (ROWS_A * COLS_A));
        }

    }
}

int main(int argc, char *argv[])
{
    struct timeval tv0, tv1, tv2;
    struct timezone tz0, tz1, tz2;

    float *A;
    cudaMallocManaged((void **)&A, ROWS_A * COLS_A * sizeof(float));

    gettimeofday(&tv0, &tz0);

    pad_cols_kernel<<<2, (ROWS_A >> 1)>>>(A);
    pad_rows_kernel<<<2, (COLS_A >> 1)>>>(A);

    dim3 dimBlock(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 dimGrid(BLOCKS_X, BLOCKS_Y);

    init_kernel<<<dimGrid, dimBlock>>>(A);

    cudaDeviceSynchronize();
    gettimeofday(&tv1, &tz1);

    grid_solver_kernel<<<dimGrid, dimBlock>>>(A);
    cudaDeviceSynchronize();

    gettimeofday(&tv2, &tz2);

    printf("Error: %f\n", global_diff / (ROWS_A * COLS_A));
    printf("Time: %ld microseconds, ", (tv2.tv_sec - tv0.tv_sec) * 1000000 + (tv2.tv_usec - tv0.tv_usec));
    printf("Init time: %ld microseconds, ", (tv1.tv_sec - tv0.tv_sec) * 1000000 + (tv1.tv_usec - tv0.tv_usec));
    printf("Compute time: %ld microseconds\n", (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec));

    cudaFree(A);
    return 0;
}
