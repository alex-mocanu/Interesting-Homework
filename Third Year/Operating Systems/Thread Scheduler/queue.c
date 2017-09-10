/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#include "queue.h"

void init_queue(queue *q)
{
	q->prev = q;
	q->next = q;
	q->val = NULL;
}

int push(queue *q, void *value)
{
	queue *aux = malloc(sizeof(queue));

	if (aux == NULL)
		return PUSH_QUEUE_FAIL;

	aux->val = value;
	aux->prev = q->prev;
	aux->next = q;
	q->prev->next = aux;
	q->prev = aux;

	return 0;
}

int push_front(queue *q, void *value)
{
	queue *aux = malloc(sizeof(queue));

	if (aux == NULL)
		return PUSH_QUEUE_FAIL;

	aux->val = value;
	aux->prev = q;
	aux->next = q->next;
	q->next->prev = aux;
	q->next = aux;

	return 0;
}

void *pop(queue *q)
{
	void *val;
	queue *aux;

	/* If the queue is void return NULL */
	if (q->next == q)
		return NULL;

	val = q->next->val;
	aux = q->next;
	aux->next->prev = q;
	q->next = aux->next;

	free(aux);
	aux = NULL;

	return val;
}

void *peek(queue *q)
{
	return q->next->val;
}
