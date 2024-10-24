#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>
#define N 1000000

int num_threads;


int main(int argc, char *argv[]) {
	pthread_t *tid;
	struct timeval tv0, tv1;
	struct timezone tz0, tz1;

	if(argc != 2) {
		printf ("Need number of threads.\n");
		exit(1);
	}
    
	num_threads = atoi(argv[1]);

    int i;

	gettimeofday(&tv0, &tz0);

	#pragma omp parallel num_threads (num_threads) private (i)
    {
        for (i=0; i<N; i++) {
            #pragma omp barrier
        }
    }

    gettimeofday(&tv1, &tz1);
    
    printf("Time: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));
}