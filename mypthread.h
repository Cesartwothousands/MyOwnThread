// File:	mypthread_t.h

// List all group members' names:
// iLab machine tested on:

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

/* in order to use the built-in Linux pthread library as a control for benchmarking, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <stdatomic.h>

typedef uint mypthread_t;

	/* add important states in a thread control block */
typedef struct threadControlBlock
{
	// YOUR CODE HERE	
	
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...
	mypthread_t t_id;
	int status;//0 = ready 1 = running 2 = blocked
	ucontext_t context;
	int priority;
} tcb;


// Feel free to add your own auxiliary data structures (linked list or queue etc...)
// blocked queue
typedef struct blocked_queue {
	tcb* threadControlBlock;
	struct queue* next;
} blocked_queue;

typedef struct run_queue {
	tcb* threadControlBlock;
	struct run_queue* next;
} run_queue;

typedef struct finished_queue {
	mypthread_t t_id;
	void* value;
	struct finished_queue* next;
} finished_queue;

/* mutex struct definition */
typedef struct mypthread_mutex_t
{

	// YOUR CODE HERE
	int locked;
	mypthread_t t_id;
	blocked_queue* blockedQueueHead;
	
} mypthread_mutex_t;



/* Function Declarations: */

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initialize a mutex */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire a mutex (lock) */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release a mutex (unlock) */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy a mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

void* myMalloc(int size);

static void sched_PSJF();

static void sched_MLFQ();

void signalHandler(int signum);

void enqueueSTCF(tcb* threadBlock);

tcb* dequeueSTCF();

void enqueueRR(tcb* threadBlock);

tcb* dequeueRR();

static void schedule();

void resetMLFQ();

void enqueueMLFQ(tcb* threadBlock);

tcb* dequeueMLFQ();

#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif

#endif
