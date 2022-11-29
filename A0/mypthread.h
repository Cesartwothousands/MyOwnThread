// File:	mypthread_t.h

// List all group members' names: Qipan Xu(qx67), Zihan Chen(zc366)
// iLab machine tested on:

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE
#define QUANTUM 15
#define LOCKED 1
#define UNLOCKED 0
#define UNTOUCHED -1

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

#define SIGSTKSZ 1024*1024

typedef uint mypthread_t;

//thread state
enum thread_state {
    THREAD_UNINIT = 0,  // uninitialized
    THREAD_RUNNABLE,    // runnable(maybe running)
    THREAD_SLEEPING,    // sleeping
    THREAD_EXIT
};

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
	enum thread_state status;//0 = ready 1 = running 2 = blocked
	ucontext_t context;
	int priority;
} tcb;


// Feel free to add your own auxiliary data structures (linked list or queue etc...)
// blocked queue


typedef struct queue {
	tcb* threadControlBlock;
	struct queue* next;
} queue;

typedef struct finished_queue {
	mypthread_t t_id;
	void* value;
	struct finished_queue* next;
} finished_queue;

/* mutex struct definition */
typedef struct mypthread_mutex_t
{
	int lock_state;
	mypthread_t t_id;
	queue* blockedQueueHead;
	
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

static void sched_PSJF();

static void sched_MLFQ();

void signalHandler(int signum);

void AddwithoutPi(tcb* threadBlock);
void AddwithPi(tcb* threadBlock);

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
