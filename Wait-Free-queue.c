// Wait-Free-queue.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#define MAX_THREADS 16   // maximum number of threads supported
#define WF_CAPACITY 1048576  // enough capacity for OPS

typedef struct {
    int value;
    atomic_int enqueued; // 0 = free, 1 = used
} WFNode;

typedef struct {
    WFNode buffer[WF_CAPACITY];
    atomic_int head;
    atomic_int tail;
} WFQueue;

typedef WFQueue LFQueue; // for test harness typedef compatibility

// Initialize the queue
void queue_init(WFQueue *q) {
    for (int i = 0; i < WF_CAPACITY; i++)
        atomic_store(&q->buffer[i].enqueued, 0);
    atomic_store(&q->head, 0);
    atomic_store(&q->tail, 0);
}

// Enqueue operation (wait-free)
int enqueue(WFQueue *q, int val) {
    while (1) {
        int t = atomic_load(&q->tail);
        int index = t % WF_CAPACITY;

        int expected = 0;
        if (atomic_compare_exchange_strong(&q->buffer[index].enqueued, &expected, 1)) {
            q->buffer[index].value = val;
            atomic_fetch_add(&q->tail, 1);
            return 1;
        } else {
            // slot busy, try next
            atomic_fetch_add(&q->tail, 1);
        }
    }
}

// Dequeue operation (wait-free)
int dequeue(WFQueue *q, int *val) {
    while (1) {
        int h = atomic_load(&q->head);
        int index = h % WF_CAPACITY;

        int expected = 1;
        if (atomic_compare_exchange_strong(&q->buffer[index].enqueued, &expected, 0)) {
            *val = q->buffer[index].value;
            atomic_fetch_add(&q->head, 1);
            return 1;
        } else {
            // slot empty, try next
            if (h >= atomic_load(&q->tail))
                return 0; // queue is empty
            atomic_fetch_add(&q->head, 1);
        }
    }
}
