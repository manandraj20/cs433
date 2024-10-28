// #include <bits/stdc++.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>
#include <pthread.h>
#include <sys/time.h>
using namespace std;
#define MAX_SIZE 10000

// Shared variables
int x = 0, y = 0;
int num_threads;

// Lamport Lock
int ticket[MAX_SIZE];
int choosing[MAX_SIZE];

// Mutex lock
pthread_mutex_t my_mutex;

int Log2(int n)
{
    int ans = 0;
    while (n != 1)
    {
        ans++;
        n >>= 1;
    }
    return ans;
}

void Acquire_Lamport(int tid)
{
    choosing[16 * tid] = 1;
    asm("mfence" ::: "memory"); // apply asm fence to make sure that choosing is stored successfully before the loads for determining the ticket number is executed
    int max = 0;
    for (int j = 0; j < num_threads; j++)
    {
        if (ticket[16 * j] > max)
        {
            max = ticket[16 * j];
        }
    }
    ticket[16 * tid] = max + 1;

    // using max_element violate the consistency
    // probably some compiler optimization is causing this issue ?
    // ticket[tid] = *max_element(ticket, ticket + num_threads) + 1;
    // asm("mfence" ::: "memory");
    asm("" ::: "memory"); // apply asm memory clobber to make sure that the compiler doesnt optimise memory accesses around the assembly block that break program logic
    choosing[16 * tid] = 0;
    asm("mfence" ::: "memory"); // Ensure "choosing" is 0 before proceeding

    for (int j = 0; j < num_threads; j++)
    {
        while (choosing[16 * j])
            ; // Wait if the other thread is choosing
        while (ticket[16 * j] != 0 &&
               (ticket[16 * j] < ticket[16 * tid] ||
                (ticket[16 * j] == ticket[16 * tid] && j < tid)))
        {
            // Wait if other thread has a higher priority
        }
    }
    return;
}

// Release function for the bakery algorithm
void Release_Lamport(int tid)
{
    asm("" ::: "memory");
    ticket[16 * tid] = 0;
    return;
}

int spinLockPtr = 0;
unsigned char CompareAndSet(int oldVal, int newVal, int *ptr)
{
    unsigned char result;
    int oldValOut;
    asm("lock cmpxchgl %4, %1 \n setzb %0"
        : "=qm"(result), "+m"(*ptr), "=a"(oldValOut)
        : "a"(oldVal), "r"(newVal)
        :);
    return result;
}

void Acquire_SpinLock()
{
    while (!CompareAndSet(0, 1, &spinLockPtr))
        ;
}

void Release_SpinLock()
{
    asm("" ::: "memory");
    spinLockPtr = 0;
}

int ttsLockPtr = 0;
void Acquire_TestAndTestAndSetLock()
{
    while (!CompareAndSet(0, 1, &ttsLockPtr))
    {
        while (ttsLockPtr)
            ;
    }
}

void Release_TestAndTestAndSetLock()
{
    asm("" ::: "memory");
    ttsLockPtr = 0;
}

int FetchAndInc(int *ptr)
{
    unsigned char result = 0;
    int oldVal, oldValOut, newVal;
    while (result != 1)
    {
        oldVal = *ptr;
        oldValOut;
        newVal = *ptr + 1;
        asm("lock cmpxchgl %4, %1 \n setzb %0"
            : "=qm"(result), "+m"(*ptr), "=a"(oldValOut)
            : "a"(oldVal), "r"(newVal)
            :);
    }

    return oldVal;
}

int _ticket = 0, release_count = 0;
/* Acquire for Ticket Lock */
void Acquire_TicketLock()
{
    int curr_ticket = FetchAndInc(&_ticket);

    while (release_count != curr_ticket)
        ;

    return;
}

/* Release for Ticket Lock */
void Release_TicketLock()
{
    asm("" ::: "memory");
    release_count++;

    return;
}

int next_ticket = 0;
unsigned char available[MAX_SIZE];
void Acquire_ArrayLock(int *ticket)
{
    *ticket = FetchAndInc(&next_ticket);

    while (!available[(*ticket % num_threads) * 64])
        ;

    return;
}

void Release_ArrayLock(int *ticket)
{
    asm("" ::: "memory");
    available[(*ticket % num_threads) * 64] = 0;
    available[((*ticket + 1) % num_threads) * 64] = 1;

    return;
}

struct bar_type
{
    int counter;
    pthread_mutex_t lock;
    int flag;
} bar_name;

void init_bar()
{
    pthread_mutex_init(&bar_name.lock, NULL);
    bar_name.counter = 0;
    bar_name.flag = 0;
}

void Rev_Sense_Barrier(int *local_sense)
{
    pthread_mutex_lock(&bar_name.lock);
    bar_name.counter++;
    if (bar_name.counter == num_threads)
    {
        pthread_mutex_unlock(&bar_name.lock);
        bar_name.counter = 0;
        bar_name.flag = *local_sense;
    }
    else
    {
        pthread_mutex_unlock(&bar_name.lock);
        while (bar_name.flag != *local_sense)
            ;
    }
    *local_sense = !(*local_sense);
}

struct Central_POSIX_Barrier
{
    int counter;
    pthread_mutex_t lock;
    pthread_cond_t cv;
} Central_Posix_barr;

/* using posix condition variable*/
void Central_POSIX_Barrier()
{
    pthread_mutex_lock(&Central_Posix_barr.lock);
    Central_Posix_barr.counter++;
    if (Central_Posix_barr.counter == num_threads)
    {
        pthread_mutex_unlock(&Central_Posix_barr.lock);
        Central_Posix_barr.counter = 0;
        pthread_cond_broadcast(&Central_Posix_barr.cv);
    }
    else
    {
        pthread_cond_wait(&Central_Posix_barr.cv, &Central_Posix_barr.lock);
        pthread_mutex_unlock(&Central_Posix_barr.lock);
    }
}

int flag[32][112]; // 16X7
int MAX;
void Tree_Barrier(int tid)
{
    unsigned int i, mask;

    for (i = 0, mask = 1; (mask & tid) != 0; ++i, mask <<= 1)
    {
        while (!flag[tid][16 * i])
            ;
        flag[tid][16 * i] = 0;
    }

    if (tid < (num_threads - 1))
    {
        flag[tid + mask][16 * i] = 1;

        while (!flag[tid][16 * (MAX + 1)])
            ;
        flag[tid][16 * (MAX + 1)] = 0;
    }

    for (mask >>= 1; mask > 0; mask >>= 1)
    {
        flag[tid - mask][16 * (MAX + 1)] = 1;
    }
}

struct Tree_CV
{
    int flag;
    pthread_mutex_t lock;
    pthread_cond_t cv;

} Tree_CV_barr[32][96];
void Tree_CV_Barrier(int tid)
{
    unsigned int i, mask;

    for (i = 0, mask = 1; (mask & tid) != 0; ++i, mask <<= 1)
    {
        pthread_mutex_lock(&Tree_CV_barr[tid][16 * i].lock);

        while (!Tree_CV_barr[tid][16 * i].flag)
            pthread_cond_wait(&Tree_CV_barr[tid][16 * i].cv, &Tree_CV_barr[tid][16 * i].lock);

        Tree_CV_barr[tid][16 * i].flag = 0;

        pthread_mutex_unlock(&Tree_CV_barr[tid][16 * i].lock);
    }

    if (tid < (num_threads - 1))
    {
        pthread_mutex_lock(&Tree_CV_barr[tid + mask][16 * i].lock);
        Tree_CV_barr[tid + mask][16 * i].flag = 1;
        pthread_cond_signal(&Tree_CV_barr[tid + mask][16 * i].cv);
        pthread_mutex_unlock(&Tree_CV_barr[tid + mask][16 * i].lock);

        pthread_mutex_lock(&Tree_CV_barr[tid][16 * (MAX + 1)].lock);
        while (!Tree_CV_barr[tid][16*(MAX + 1)].flag)
            pthread_cond_wait(&Tree_CV_barr[tid][16 * (MAX + 1)].cv, &Tree_CV_barr[tid][16 * (MAX + 1)].lock);
        Tree_CV_barr[tid][16 * (MAX + 1)].flag = 0;
        pthread_mutex_unlock(&Tree_CV_barr[tid][16 * (MAX + 1)].lock);
    }
    for (mask >>= 1; mask > 0; mask >>= 1)
    {
        pthread_mutex_lock(&Tree_CV_barr[tid - mask][16 * (MAX + 1)].lock);
        Tree_CV_barr[tid - mask][16 * (MAX + 1)].flag = 1;
        pthread_cond_signal(&Tree_CV_barr[tid - mask][16 * (MAX + 1)].cv);
        pthread_mutex_unlock(&Tree_CV_barr[tid - mask][16 * (MAX + 1)].lock);
    }
}