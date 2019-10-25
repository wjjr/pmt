/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_QUEUE_H
#define _PMT_QUEUE_H

#include <stdlib.h>

struct queue {
    void *pointer;
    struct queue *tail;
    struct queue *next;
    struct queue *prev;
};

static __inline __attribute__((unused)) struct queue *queue_push(struct queue *q, void *const p) {
    struct queue *n = malloc(sizeof(struct queue));

    if (q == NULL)
        q = calloc(1, sizeof(struct queue));

    n->pointer = p;
    n->tail = NULL;
    n->prev = q;
    n->next = q->next;
    q->next = n;

    if (n->next != NULL)
        n->next->prev = n;

    if (q->tail == NULL)
        q->tail = n;

    return q;
}

static __inline __attribute__((unused)) void *queue_pop(struct queue *q) {
    void *s = NULL;
    struct queue *q_tail = q->tail;

    if (q_tail != NULL) {
        s = q_tail->pointer;

        if (q_tail->prev != q) {
            q->tail = q_tail->prev;
            q->tail->next = NULL;
        } else {
            q->tail = q->next = NULL;
        }

        free(q_tail);
    } else
        free(q);

    return s;
}

#endif /* _PMT_QUEUE_H */
