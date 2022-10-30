// File:	mypthread_t.h

// List all group members' names:Qipan Xu; Zihan Chen
// iLab machine tested on:


#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H


#define _GNU_SOURCE
#define LOCKED 0
#define UNLOCKED 1
#define MEM 12800

/* in order to use the built-in Linux pthread library as a control for benchmarking,
 * you have to comment the USE_MYTHREAD macro */
//#define USE_MYTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>


typedef uint mypthread_t;

enum thread_state {
    THREAD_UNINIT = 0,  // uninitialized
    THREAD_SLEEPING,    // sleeping
    THREAD_RUNNABLE,    // runnable(maybe running)
    THREAD_EXIT
};

	/* add important states in a thread control block */
typedef struct threadControlBlock
{
	// thread Id
    unsigned int tid;
	// thread status
    enum thread_state state;
	// thread context & stack inside
    ucontext_t context;
	// thread priority
    unsigned int priority;
	// thread ptr that creates this thread
    struct threadControlBlock *parent;
    struct threadControlBlock* next;
    // function execution
    void *(*function)(void*);
    void * arg;
    // return value after pthread_exit
    void *value_ptr;
} tcb;


/* mutex struct definition */
typedef struct mypthread_mutex_t
{
    //pthread_mutex_t t;
    int owner_id;
    int mutex_id;
    int lock_state;
} mypthread_mutex_t;


// Feel free to add your own auxiliary data structures (linked list or queue etc...)
struct ThreadQueue{
    tcb* head;
    tcb* tail;
    unsigned int size;
};

void addThread(struct ThreadQueue* T, tcb *a){
    if (T->size == 0){
        T->head = a;
        T->tail = a;
    }
    else{
        T->tail->next = a;
        T->tail = T->tail->next;
    }
    T->size++;
};

tcb* removeThread(struct ThreadQueue* T){
    if (T->size != 0){
        tcb* res = T->head;
        T->head = T->head->next;
        T->size--;
        return res;
    }
    return NULL;
}

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
