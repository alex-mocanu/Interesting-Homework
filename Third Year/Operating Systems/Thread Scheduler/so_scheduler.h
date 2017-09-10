/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#ifndef SO_SCHEDULER_H_
#define SO_SCHEDULER_H_

#include "utils.h"

/* OS dependent stuff */
#ifdef __linux__
#include <pthread.h>

#define DECL_PREFIX

typedef pthread_t tid_t;
#elif defined(_WIN32)
#include <windows.h>

#ifdef DLL_IMPORTS
#define DECL_PREFIX __declspec(dllimport)
#else
#define DECL_PREFIX __declspec(dllexport)
#endif

typedef DWORD tid_t;
#else
#error "Unknown platform"
#endif

/*
 * return value of failed tasks
 */
#define INVALID_TID ((tid_t)0)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * creates and initializes scheduler
 * + time quantum for each thread
 * + number of IO devices supported
 * returns: 0 on success or negative on error
 */
DECL_PREFIX int so_init(unsigned int time_quant, unsigned int io);

/*
 * creates a new so_task_t and runs it according to the scheduler
 * + handler function
 * + priority
 * returns: tid of the new task if successful or INVALID_TID
 */
DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority);

/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned int io);

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned int io);

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec(void);

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end(void);

#ifdef __cplusplus
}
#endif

#endif /* SO_SCHEDULER_H_ */

