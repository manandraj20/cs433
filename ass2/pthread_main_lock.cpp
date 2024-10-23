#include "custom_lock.cpp"
using namespace std;

void *work(void *arg)
{
    int tid = *(int *)arg;

    for (int i = 0; i < N; i++)
    {
        Acquire_Lamport(tid);
        // pthread_mutex_lock(&my_mutex);
        // Acquire_SpinLock();
        // Acquire_TTS();
        // Acquire_TicketLock();
        // Acquire_ArrayLock(&array_lock_ticket);
        // Acquire_POSIX_mutex();
        // Acquire_Binary_Semaphore();

        // Critical section
        assert(x == y);
        x = y + 1;
        y++;

        Release_Lamport( tid);
        // pthread_mutex_unlock(&my_mutex);
        // Release_SpinLock();
        // Release_TTS();
        // Release_TicketLock();
        // Release_ArrayLock(&array_lock_ticket);
        // Release_POSIX_mutex();
        // Release_Binary_Semaphore();
    }
}

int main(int argc, char *argv[])
{
    int i, j;
    pthread_mutex_init(&my_mutex, NULL);
    pthread_t *threads;
    pthread_attr_t attr;
    if (argc != 2)
    {
        printf("Need number of threads.\n");
        exit(1);
    }
    num_threads = atoi(argv[1]);

    struct timeval tp_start, tp_end;

    threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    assert(threads != NULL);
    int tid[num_threads];
 	for(int i = 0; i < num_threads; i++) tid[i] = i;
    pthread_attr_init(&attr);
    gettimeofday(&tp_start, NULL);

    for (i = 1; i < num_threads; i++)
    {
        /* pthread_create arguments: thread pointer,
                                     attribute pointer,
                                     function pointer,
                                     argument pointer to the function
        */
        pthread_create(&threads[i], &attr, work, &tid[i]);
    }

    for (int i = 0; i < N; i++)
    {
        Acquire_Lamport(0);
        // pthread_mutex_lock(&my_mutex);
        // Acquire_SpinLock();
        // Acquire_TTS();
        // Acquire_TicketLock();
        // Acquire_ArrayLock(&array_lock_ticket);
        // Acquire_POSIX_mutex();
        // Acquire_Binary_Semaphore();
        assert(x == y);
        x = y + 1;
        y++;
        Release_Lamport(0);
        // pthread_mutex_unlock(&my_mutex);
        // Release_SpinLock();
        // Release_TTS();
        // Release_TicketLock();
        // Release_ArrayLock(&array_lock_ticket);
        // Release_POSIX_mutex();
        // Release_Binary_Semaphore();
    }

    for (i = 1; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&tp_end, NULL);

    printf("Total time: %ld microseconds\n", tp_end.tv_sec * 1000000 + tp_end.tv_usec - (tp_start.tv_sec * 1000000 + tp_start.tv_usec));
    printf("x = %d, y = %d\n", x, y);
    assert(x == N * num_threads);
    return 0;
}