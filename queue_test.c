#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

/* Include the right queue implementation based on compile-time flag */
#ifdef LockFree
#include "Lock-Free-queue.c"
typedef LFQueue QueueType;
#elif defined(WaitFree)
#include "Wait-Free-queue.c"
typedef WFQueue QueueType;
#elif defined(LockBased)
#include "LockBased-queue.c"
typedef LBQueue QueueType;
#else
#error "Must define a queue implementation using -DLockFree, -DWaitFree, or -DLockBased"
#endif

#define OPS 1000000
#define THREADS 16

QueueType Q;
volatile int start_flag = 0;

/* Thread barrier */
void wait_start() {
    __sync_fetch_and_add(&start_flag, 1);
    while (start_flag < THREADS);
}

/* ---------- Test 1: Concurrent Enqueue ---------- */
void *test_enqueue(void *arg) {
    wait_start();
    long tid = (long)arg;
    int start = (OPS / THREADS) * tid;
    int end   = start + (OPS / THREADS);

    for (int i = start; i < end; i++) {
        enqueue(&Q, i);
    }
    return NULL;
}

/* ---------- Test 2: Concurrent Dequeue ---------- */
void *test_dequeue(void *arg) {
    wait_start();
    int dummy;
    for (int i = 0; i < OPS / THREADS; i++) {
        while (!dequeue(&Q, &dummy));
    }
    return NULL;
}

/* ---------- Test 3: Mixed ---------- */
void *test_mixed(void *arg) {
    wait_start();
    long tid = (long)arg;
    int start = (OPS/2 / THREADS) * tid;

    for (int i = 0; i < OPS / THREADS; i++) {
        if (i % 2 == 0)
            enqueue(&Q, start + i);
        else {
            int tmp;
            while (!dequeue(&Q, &tmp));
        }
    }
    return NULL;
}

double run_test(void *(*fn)(void *)) {
    pthread_t th[THREADS];
    struct timeval t1, t2;

    start_flag = 0;
    gettimeofday(&t1, NULL);

    for (long i = 0; i < THREADS; i++)
        pthread_create(&th[i], NULL, fn, (void *)i);

    for (int i = 0; i < THREADS; i++)
        pthread_join(th[i], NULL);

    gettimeofday(&t2, NULL);

    double seconds = (t2.tv_sec - t1.tv_sec)
                   + (t2.tv_usec - t1.tv_usec) / 1e6;
    return seconds;
}

int main() {
#ifdef LockFree
    printf("Lock-Free Queue Tests\n\n");
#elif defined(WaitFree)
    printf("Wait-Free Queue Tests\n\n");
#elif defined(LockBased)
    printf("Lock-Based Queue Tests\n\n");
#endif

    queue_init(&Q);

    printf("Test 1: 1M concurrent enqueues\n");
    double t1 = run_test(test_enqueue);
    printf("Throughput: %.2f ops/sec\n\n", OPS / t1);

    printf("Test 2: 1M concurrent dequeues\n");
    double t2 = run_test(test_dequeue);
    printf("Throughput: %.2f ops/sec\n\n", OPS / t2);

    printf("Test 3: Mixed enqueue/dequeue\n");
    double t3 = run_test(test_mixed);
    printf("Throughput: %.2f ops/sec\n", OPS / t3);

    return 0;
}
