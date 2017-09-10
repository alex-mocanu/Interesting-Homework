/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <pthread.h>
#include "queue.h"

/*
 * the maximum priority that can be assigned to a thread
 */
#define SO_MAX_PRIO 5
/*
 * the maximum number of events
 */
#define SO_MAX_NUM_EVENTS 256

/*
 * handler prototype
 */
typedef void (so_handler)(unsigned int);

/*
 * barrier
 */
typedef struct {
	int threshold;
	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
} barrier;

/*
 * paramaters passed to a threads start function
 */
typedef struct {
	so_handler *func;
	unsigned int priority;
	pthread_cond_t *cond;
	barrier *barr;
} thread_params;

/*
 * thread properties
 */
typedef struct {
	pthread_t thread;
	int priority;
	pthread_cond_t *cond;
} thread;

/* queue of threads */
queue threads;
/* queues of ready threads by priority */
queue ready[SO_MAX_PRIO + 1];
/* queues of threads waiting after events */
queue waiting[SO_MAX_NUM_EVENTS];
/* running thread */
thread *running;
/* time spent by current thread on CPU */
int tim;
/* time quantum */
int time_quantum;
/* number of io events */
int io_events;
/* mutex */
pthread_mutex_t mutex;

/* Reset time */
void reset_time(void);

/*
 * Increment time
 */
void tick(void);

/*
 * Preempt running thread if necessary
 */
void preempt(void);

/*
 * Start thread
 */
void *start_thread(void *param);

/*
 * Finds the process to let on CPU
 */
thread *thread_to_run(void);

/*
 * Initialize barrier
 */
void barrier_init(barrier *b, int threshold);

/*
 * Destroy barrier
 */
void barrier_destroy(barrier *b);

/*
 * Barrier wait
 */
void barrier_wait(barrier *b);

#endif
