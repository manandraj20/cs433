#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/time.h>
using namespace std;
#define MAX_SIZE 10000

// Shared variables
int x = 0, y = 0;
int N = 1e7; // 10^7 iterations
int num_threads;

// Lamport Lock
int ticket[MAX_SIZE];
int choosing[MAX_SIZE];

// Mutex lock
pthread_mutex_t my_mutex;


void Acquire_Lamport(int tid)
{
    choosing[16*tid] = 1;
    asm("mfence" ::: "memory"); // apply asm fence to make sure that choosing is stored successfully before the loads for determining the ticket number is executed
    int max = 0;
    for (int j = 0; j < num_threads; j++)
    {
        if (ticket[16*j] > max)
        {
            max = ticket[16*j];
        }
    }
    ticket[16*tid] = max + 1;

    // using max_element violate the consistency
    // probably some compiler optimization is causing this issue ?
    // ticket[tid] = *max_element(ticket, ticket + num_threads) + 1;

    asm("" ::: "memory"); // apply asm memory clobber to make sure that the compiler doesnt optimise memory accesses around the assembly block that break program logic
    choosing[16*tid] = 0;
    asm("mfence" ::: "memory"); // Ensure "choosing" is 0 before proceeding

    for (int j = 0; j < num_threads; j++)
    {
        while (choosing[16*j])
            ; // Wait if the other thread is choosing
        while (ticket[16*j] != 0 &&
               (ticket[16*j] < ticket[16*tid] ||
                (ticket[16*j] == ticket[16*tid] && j < tid)))
        {
            // Wait if other thread has a higher priority
        }
    }
    return;
}

// Release function for the bakery algorithm
void Release_Lamport( int tid)
{
    asm("" ::: "memory");
    ticket[16*tid] = 0;
    return;
}



