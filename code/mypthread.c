// File:	mypthread.c

// List all group members' names: Qipan Xu(qx67), Zihan Chen(zc366)
// iLab machine tested on:


#include "mypthread.h"
#include <pthread.h>


// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

struct ThreadQueue waitQueue;
struct ThreadQueue readyQueue;
tcb* t_remove = NULL;

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{	
	// create a Thread Control Block
	tcb* t;
    t->state = THREAD_UNINIT;
    t->tid = *(thread);

	// create and initialize the context of this thread
	getcontext(&t->context);

	// allocate heap space for this thread's stack
	t->context.uc_stack.ss_sp = malloc(MEM);
    t->context.uc_stack.ss_size = MEM;

	// execute the specified function
    t->function = function;
	t->arg = arg;
    t->function(t->arg);

	// after everything is all set, push this thread into the ready queue
    addThread(&readyQueue, t);
	thread = (mypthread_t *) t;
    
	return 0;	
};

Zihan
/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield()
{
	// change current thread's state from Running to Ready
	// save context of this thread to its thread control block
	// switch from this thread's context to the scheduler's context
	
	No timer and no differenr status of thread
	//setitimer(ITIMER_PROF, &timerOff, NULL);
	//swapcontext(&(currentThread->context), &schedulerContext);
	return 0;
};

如何想到ucontext？
/* terminate a thread */
void mypthread_exit(void *value_ptr)
{
	// preserve the return value pointer if not NULL
	// deallocate any dynamic memory allocated when starting this thread
    t_remove = removeThread(&readyQueue);
    t_remove->value_ptr = value_ptr;
    t_remove->state = THREAD_EXIT;
	
    free(t_remove->context.uc_stack.ss_sp);
    t_remove->context.uc_stack.ss_size = 0;
	
	
	//return;
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr)
{
	// wait for a specific thread to terminate
	while(t_remove == NULL);

	// deallocate any dynamic memory created by the joining thread
	value_ptr = t_remove->value_ptr;

	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	//initialize data structures for this mutex

	mutex = myMalloc(sizeof(mypthread_mutex_t));
	__atomic_clear(&mutex->lock_state,__ATOMIC_RELAXED);
	mutex->mutex_id = -1;
	mutex->blockedQueueHead = NULL;
	return 0;
};

How to decide currentThread and timer
/* aquire a mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex)
{
	// use the built-in test-and-set atomic function to test the mutex
	// if the mutex is acquired successfully, return
	// if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
	
	if (__atomic_test_and_set(&mutex->lock_state,__ATOMIC_RELAXED)==1){
		setitimer(ITIMER_PROF, &timerOff, NULL);

		blocked_queue* newBlockedNode = myMalloc(sizeof(blocked_queue));
		newBlockedNode->threadControlBlock = currentThread;
		newBlockedNode->threadControlBlock->state = 2;
		newBlockedNode->next = NULL;

		currentThread = NULL;

		if (mutex->blockedQueueHead == NULL)
		{
			mutex->blockedQueueHead = newBlockedNode;
		}
		else
		{
			blocked_queue* crnt = mutex->blockedQueueHead;

			while (crnt->next != NULL)
			{
				crnt = crnt->next;
			}

			crnt->next = newBlockedNode;
		}

		swapcontext(&(newBlockedNode->threadControlBlock->context), &schedulerContext);
	}
	
	mutex->lock_state = 1;
	mutex->mutex_id = currentThread->mutex_id;

	return 0;
};

How to decide currentThread
/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex)
{
	// update the mutex's metadata to indicate it is unlocked
	// put the thread at the front of this mutex's blocked/waiting queue in to the run queue

	if (mutex->lock_state == 1 && mutex->mutex_id == currentThread->mutex_id)
	{
		__atomic_clear(&mutex->lock_state,__ATOMIC_RELAXED);
		mutex->mutex_id = -1;

		blocked_queue* crnt = mutex->blockedQueueHead;

		//take all blocked threads on this mutex and enqueue into runQueue
		while (crnt != NULL)
		{
			#ifndef MLFQ
				enqueueSTCF(crnt->threadControlBlock);
			#else
				enqueueMLFQ(crnt->threadControlBlock);
			#endif

			crnt = crnt->next;
		}
		mutex->blockedQueueHead = NULL;
	}
	return -1;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex)
{
	// deallocate dynamic memory allocated during mypthread_mutex_init

/*
struct obj {
pthread_mutex_t om;
    int refcnt;
    ...
};


obj_done(struct obj *op)
{
    pthread_mutex_lock(&op->om);
    if (--op->refcnt == 0) {
        pthread_mutex_unlock(&op->om);
(A)     pthread_mutex_destroy(&op->om);
(B)     free(op);
    } else
(C)     pthread_mutex_unlock(&op->om);
}
*/
	
	return 0;
};

/* scheduler */
static void schedule()
{
	// YOUR CODE HERE
	
	// each time a timer signal occurs your library should switch in to this context
	
	// be sure to check the SCHED definition to determine which scheduling algorithm you should run
	//   i.e. RR, PSJF or MLFQ

	return;
}

/* Round Robin scheduling algorithm */
static void sched_RR()
{
	// YOUR CODE HERE
	
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
	
	return;
}

/* Preemptive PSJF (STCF) scheduling algorithm */
static void sched_PSJF()
{
	// YOUR CODE HERE

	// Your own implementation of PSJF (STCF)
	// (feel free to modify arguments and return types)

	return;
}

/* Preemptive MLFQ scheduling algorithm */
/* Graduate Students Only */
static void sched_MLFQ() {
	// YOUR CODE HERE
	
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	return;
}

// Feel free to add any other functions you need

// YOUR CODE HERE
