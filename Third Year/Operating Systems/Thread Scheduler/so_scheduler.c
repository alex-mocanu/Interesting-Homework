/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#include "so_scheduler.h"

int so_init(unsigned int time_quant, unsigned int io)
{
	int i;

	/* Do sanity checks */
	if (time_quant == 0)
		return INVALID_TIME_QUANTUM;
	if (time_quantum != 0)
		return MULTIPLE_INITS;
	if (io > SO_MAX_NUM_EVENTS)
		return INVALID_EVENTS_NUMBER;

	/* Initialize time quantum */
	time_quantum = time_quant;
	/* Set running thread */
	running = NULL;
	/* Initialize number of io events */
	io_events = io;

	/* Initialize queue of threads*/
	init_queue(&threads);

	/* Initialize ready queues */
	for (i = 0; i <= SO_MAX_PRIO; ++i)
		init_queue(&ready[i]);

	/* Initialize event queues */
	for (i = 0; i < io_events; ++i)
		init_queue(&waiting[i]);

	/* Initialize mutex */
	pthread_mutex_init(&mutex, NULL);

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	int rc;
	pthread_t tid;
	thread *current_thread, *th;
	thread_params *aux;

	/* Increase time */
	tick();

	/* Invalid handler */
	if (func == NULL)
		return INVALID_TID;

	/* Invalid priority */
	if (priority > SO_MAX_PRIO)
		return INVALID_TID;

	/* Create new thread */
	aux = malloc(sizeof(thread_params));
	if (aux == NULL)
		return INVALID_TID;
	aux->func = func;
	aux->priority = priority;
	aux->cond = malloc(sizeof(pthread_cond_t));
	if (aux->cond == NULL)
		return INVALID_TID;
	aux->barr = malloc(sizeof(barrier));
	if (aux->barr == NULL)
		return INVALID_TID;
	pthread_cond_init(aux->cond, NULL);
	barrier_init(aux->barr, 2);
	rc = pthread_create(&tid, NULL, start_thread, aux);
	if (rc < 0)
		return INVALID_TID;

	th = malloc(sizeof(thread));
	if (th == NULL)
		return INVALID_TID;
	th->thread = tid;
	th->priority = priority;
	th->cond = aux->cond;

	/* Put thread in thread queue*/
	push(&threads, th);

	barrier_wait(aux->barr);
	/* If there is no running thread, let it on CPU */
	if (running == NULL) {
		running = th;
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(th->cond);
		reset_time();
		pthread_mutex_unlock(&mutex);
	}
	/* If created thread has better priority, let it on CPU */
	else if (running->priority < priority) {
		current_thread = running;
		push(ready + current_thread->priority, current_thread);
		running = th;
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(th->cond);
		reset_time();

		/* Wait to re-enter CPU */
		pthread_cond_wait(current_thread->cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}
	/* Else put created thread in ready queue */
	else {
		pthread_mutex_lock(&mutex);
		push(ready + priority, th);
		pthread_mutex_unlock(&mutex);
		/* Preempt running thread if necessary */
		preempt();
	}

	barrier_destroy(aux->barr);
	free(aux);
	return tid;
}

int so_wait(unsigned int io)
{
	thread *current_thread;

	/* Event is invalid */
	if (io >= io_events || io < 0)
		return INVALID_EVENT;

	/* Increase time */
	tick();

	pthread_mutex_lock(&mutex);
	/* Transition from running to waiting state */
	current_thread = running;
	push(waiting + io, current_thread);

	/* Let another process on the CPU */
	running = thread_to_run();
	reset_time();
	pthread_cond_signal(running->cond);

	/* Wait to run on CPU */
	pthread_cond_wait(current_thread->cond, &mutex);
	pthread_mutex_unlock(&mutex);

	return 0;
}

int so_signal(unsigned int io)
{
	int waiting_counter = 0;
	thread *th, *current_thread = NULL;

	/* Event is invalid */
	if (io >= io_events || io < 0)
		return INVALID_EVENT;

	/* Increase time */
	tick();

	current_thread = running;
	th = (thread *)pop(waiting + io);
	/* Move waiting threads to ready queues or running state & count them */
	while (th != NULL) {
		++waiting_counter;
		/* Put thread in running if it has better priority */
		if (running->priority < th->priority) {
			push(ready + running->priority, running);
			running = th;
			reset_time();
		}
		/* Else put in ready state*/
		else
			push(ready + th->priority, th);

		th = (thread *)pop(waiting + io);
	}

	/* Wake-up new running thread waiting on event */
	if (current_thread != running) {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(running->cond);
		pthread_cond_wait(current_thread->cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}
	/* Preempt thread */
	else
		preempt();

	return waiting_counter;
}

void so_exec(void)
{
	/* Increase time */
	tick();

	/* Preempt thread if necessary */
	preempt();
}

void so_end(void)
{
	thread *th;

	/* Exit if there was no init */
	if (time_quantum == 0)
		return;

	/* Wait for all threads to finish */
	th = (thread *)pop(&threads);
	while (th != NULL) {
		pthread_join(th->thread, NULL);
		/* Free memory */
		pthread_cond_destroy(th->cond);
		free(th->cond);
		free(th);
		th = (thread *)pop(&threads);
	}

	/* Reset time quantum */
	time_quantum = 0;
}
