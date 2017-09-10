/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#include "utils.h"
#include <stdio.h>

void reset_time(void)
{
	tim = 0;
}

void tick(void)
{
	++tim;
}

void preempt(void)
{
	thread *next_thread, *current_thread;

	if (tim == time_quantum) {
		current_thread = running;
		next_thread = thread_to_run();
		/* Evict current thread if there is a better one */
		if (next_thread != NULL &&
		next_thread->priority >= current_thread->priority) {
			running = next_thread;
			reset_time();
			push(ready + current_thread->priority, current_thread);

			pthread_mutex_lock(&mutex);
			pthread_cond_signal(running->cond);
			pthread_cond_wait(current_thread->cond,
				&mutex);
			pthread_mutex_unlock(&mutex);
		}
		/* Else keep current thread */
		else {
			/* Put back extracted thread */
			if (next_thread != NULL)
				push_front(ready + next_thread->priority,
					next_thread);
			reset_time();
		}
	}
}

void *start_thread(void *param)
{
	/* Extract parameters */
	thread_params *aux = (thread_params *)param;
	so_handler *func = aux->func;
	unsigned int priority = aux->priority;
	pthread_cond_t *cond = aux->cond;
	barrier *barr = aux->barr;

	/* Wait to get on the CPU */
	pthread_mutex_lock(&mutex);
	barrier_wait(barr);
	pthread_cond_wait(cond, &mutex);
	pthread_mutex_unlock(&mutex);

	/* Run handler */
	func(priority);
	/* Get out of the CPU */
	running = thread_to_run();
	reset_time();
	/* Signal the next thread if there is still any */
	if (running != NULL)
		pthread_cond_signal(running->cond);

	return NULL;
}

thread *thread_to_run(void)
{
	int prio = SO_MAX_PRIO;

	while (ready[prio].next == ready + prio) {
		--prio;
		if (prio < 0)
			return NULL;
	}

	return (thread *)pop(ready + prio);
}

void barrier_init(barrier *b, int threshold)
{
	b->threshold = threshold;
	b->cond = malloc(sizeof(pthread_cond_t));
	b->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_cond_init(b->cond, NULL);
	pthread_mutex_init(b->mutex, NULL);
}

void barrier_destroy(barrier *b)
{
	pthread_cond_destroy(b->cond);
	pthread_mutex_destroy(b->mutex);
	free(b->cond);
	free(b->mutex);
	free(b);
}

void barrier_wait(barrier *b)
{
	static int count;

	pthread_mutex_lock(b->mutex);
	++count;
	if (count == b->threshold) {
		count = 0;
		pthread_cond_broadcast(b->cond);
	} else
		pthread_cond_wait(b->cond, b->mutex);
	pthread_mutex_unlock(b->mutex);
}
