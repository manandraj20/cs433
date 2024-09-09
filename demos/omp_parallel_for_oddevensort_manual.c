#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <assert.h>

#define SIZE (1 << 17)

int main (int argc, char *argv[])
{
	int i, temp, phase, tid;
	int start_even, start_odd, end_even, end_odd;
	struct timeval tv0, tv1;
	struct timezone tz0, tz1;
	int nthreads;
	int *a = (int*)malloc(SIZE*sizeof(int));
	assert(a != NULL);

	if (argc != 2) {
		printf ("Need number of threads.\n");
		exit(1);
	}
	nthreads = atoi(argv[1]);

	for (i=0; i<SIZE; i++) a[i] = SIZE - i;

	gettimeofday(&tv0, &tz0);

#pragma omp parallel num_threads (nthreads) private(phase,i,temp,tid,start_even,start_odd,end_even,end_odd)
	{
		tid = omp_get_thread_num();
		start_even = tid*((SIZE-1)/nthreads)+1;
		start_odd = tid*((SIZE-2)/nthreads)+1;
		start_even = ((start_even-1) % 2) ? start_even+1 : start_even;
		start_odd = ((start_odd-1) % 2) ? start_odd+1 : start_odd;
		if (tid == (nthreads - 1)) {
			end_even = SIZE;
			end_odd = SIZE-1;
		}
		else {
			end_even = (tid+1)*((SIZE-1)/nthreads)+1;
			end_odd = (tid+1)*((SIZE-2)/nthreads)+1;
		}
		for (phase=0; phase<SIZE; phase++) {
   			if ((phase % 2) == 0) {  // Even phase
      				for (i=start_even; i<end_even; i+=2) {
         				if (a[i-1] > a[i]) {
						temp = a[i-1];
						a[i-1] = a[i];
						a[i] = temp;
					}
				}					
      			}
   			else {  // Odd phase
       				for (i=start_odd; i<end_odd; i+=2) {
          				if (a[i] > a[i+1]) {
						temp = a[i];
						a[i] = a[i+1];
						a[i+1] = temp;
					}
				}
       			}
#pragma omp barrier
   		}
	}

	gettimeofday(&tv1, &tz1);

	for (i=0; i<SIZE-1; i++) {
		if (a[i] > a[i+1]) printf("Error at index %d\n", i);
	}
	printf("\nTime: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));
	return 0;
}