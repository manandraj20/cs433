#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <omp.h>

using namespace std;
int main(int argc, char *argv[]) {
    struct timeval tv0, tv1;
	struct timezone tz0, tz1;
    long long N=1000000000;
    int nthreads = stoi(argv[1]);
    cout<<nthreads<<endl;
    long long a=0;
    int localsum[10*nthreads]={0};
    gettimeofday(&tv0, &tz0);
    # pragma omp parallel for num_threads(nthreads)
    for(long long i=0;i<N;i++){
      int tid = omp_get_thread_num();
      localsum[10*tid]+=i;   
    }
    gettimeofday(&tv1, &tz1);
    long long sum=0;
    for(int i=0;i<10*nthreads;i+=10){
        sum+=localsum[i];
    }
    cout<<sum<<endl;
    printf("Time: %ld microseconds\n", (tv1.tv_sec-tv0.tv_sec)*1000000+(tv1.tv_usec-tv0.tv_usec));
    return 0;

}