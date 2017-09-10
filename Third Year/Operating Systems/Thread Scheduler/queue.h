/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 *
 * 331CB
 */

#ifndef SO_QUEUE_H_
#define SO_QUEUE_H_

#include <stdlib.h>
#include "error_codes.h"

typedef struct queue {
	struct queue *prev;
	struct queue *next;
	void *val;
} queue;

/*
 * Initialize queue
 */
void init_queue(queue *q);

/*
 * Insert an element in the queue
 */
int push(queue *q, void *value);

/*
 * Put element in front of the queue
 */
int push_front(queue *q, void *value);

/*
 * Extract head of the queue
 */
void *pop(queue *q);

#endif
