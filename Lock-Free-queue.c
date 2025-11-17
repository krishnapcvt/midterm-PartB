#ifndef LF_QUEUE_H
#define LF_QUEUE_H

#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>

/* -------- Node structure -------- */
typedef struct Node {
    int value;
    _Atomic(struct Node *) next;
} Node;

/* -------- Queue structure -------- */
typedef struct {
    _Atomic(Node *) head;
    _Atomic(Node *) tail;
} LFQueue;

/* --------- Create a new node --------- */
static inline Node *new_node(int value) {
    Node *n = malloc(sizeof(Node));
    n->value = value;
    atomic_store(&n->next, NULL);
    return n;
}

/* --------- Initialize queue --------- */
static inline void queue_init(LFQueue *q) {
    Node *sentinel = new_node(-1);
    atomic_store(&q->head, sentinel);
    atomic_store(&q->tail, sentinel);
}

/* --------- Lock-free enqueue --------- */
static inline void enqueue(LFQueue *q, int value) {
    Node *node = new_node(value);

    while (1) {
        Node *tail = atomic_load(&q->tail);
        Node *next = atomic_load(&tail->next);

        if (tail == atomic_load(&q->tail)) {
            if (next == NULL) {
                if (atomic_compare_exchange_weak(&tail->next, &next, node)) {
                    atomic_compare_exchange_weak(&q->tail, &tail, node);
                    return;
                }
            } else {
                atomic_compare_exchange_weak(&q->tail, &tail, next);
            }
        }
    }
}

/* --------- Lock-free dequeue --------- */
static inline bool dequeue(LFQueue *q, int *result) {
    while (1) {
        Node *head = atomic_load(&q->head);
        Node *tail = atomic_load(&q->tail);
        Node *next = atomic_load(&head->next);

        if (head == atomic_load(&q->head)) {
            if (next == NULL) {
                return false;       // queue empty
            }
            if (head == tail) {
                atomic_compare_exchange_weak(&q->tail, &tail, next);
            } else {
                int value = next->value;
                if (atomic_compare_exchange_weak(&q->head, &head, next)) {
                    free(head);
                    *result = value;
                    return true;
                }
            }
        }
    }
}

#endif
