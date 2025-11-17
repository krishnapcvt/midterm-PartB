// Lock-Based-queue.c
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "ttas_spinlock.h"

typedef struct LBNode {
    int value;
    struct LBNode *next;
} LBNode;

typedef struct {
    LBNode *head;
    LBNode *tail;
    spinlock lock;
} LBQueue;

// Initialize queue
void queue_init(LBQueue *q) {
    LBNode *dummy = malloc(sizeof(LBNode));
    dummy->next = NULL;
    q->head = dummy;
    q->tail = dummy;
    // pthread_mutex_init(&q->lock, NULL);
}

// Enqueue
void enqueue(LBQueue *q, int value) {
    LBNode *node = malloc(sizeof(LBNode));
    node->value = value;
    node->next = NULL;

    spin_lock(&q->lock);
    q->tail->next = node;
    q->tail = node;
    spin_unlock(&q->lock);
}

// Dequeue
int dequeue(LBQueue *q, int *value) {
    spin_lock(&q->lock);
    LBNode *head = q->head;
    LBNode *next = head->next;

    if (next == NULL) {
        spin_unlock(&q->lock);
        return 0; // queue empty
    }

    *value = next->value;
    q->head = next;
    spin_unlock(&q->lock);

    free(head);
    return 1;
}
