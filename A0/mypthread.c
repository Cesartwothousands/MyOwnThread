// File:	mypthread.c

// List all group members' names: Qipan Xu(qx67), Zihan Chen(zc366)
// iLab machine tested on:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

queue* List[6];
queue* runQueueHead = NULL;
finished_queue* finishedQueueHead = NULL;
int finish = 0, init = 1, count = 0, list_timer = 0;

struct itimerval timer;
struct itimerval timerOff;

tcb* currentThread = NULL;
tcb* mainThread = NULL;
ucontext_t schedulerContext, mainContext;

/* create a new thread */
void* runner(void*(*function)(void*), void* arg)
{
    function(arg);
}


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
	   // create a Thread Control Block
	   // create and initialize the context of this thread
	   // allocate heap space for this thread's stack
	   // after everything is all set, push this thread into the ready queue
	if (init){
		mainThread = malloc(sizeof(tcb));
		mainThread->t_id = count++;
		mainThread->status = THREAD_UNINIT;
		mainThread->priority = 0;

		getcontext(&mainContext);

		mainThread->context = mainContext;

		getcontext(&schedulerContext);
		void* scheduleStack = malloc(SIGSTKSZ);
		schedulerContext.uc_link = NULL;
		schedulerContext.uc_stack.ss_sp = scheduleStack;
		schedulerContext.uc_stack.ss_size = SIGSTKSZ;
		schedulerContext.uc_stack.ss_flags = 0;
		makecontext(&schedulerContext, (void*)&schedule, 0);
	}

	tcb* newThread = malloc(sizeof(tcb));
	newThread->t_id = count++;
	*thread = newThread->t_id;
	newThread->status = THREAD_UNINIT;
	newThread->priority = 0;

	ucontext_t current;

	void* stack = malloc(SIGSTKSZ);
	current.uc_link = NULL;
	current.uc_stack.ss_sp = stack;
	current.uc_stack.ss_size = SIGSTKSZ;
	current.uc_stack.ss_flags = 0;
	makecontext(&current, (void*)&runner, 2, function, arg);
	newThread->context = current;

	if (init){
		init = 0;

		struct sigaction signal;
		memset(&signal,0,sizeof(signal));
		signal.sa_handler = &signalHandler;
		sigaction(SIGPROF, &signal, NULL);

		timer.it_interval.tv_usec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = 15;
		timer.it_value.tv_sec = 0;

		timerOff.it_interval.tv_usec = 0; 
		timerOff.it_interval.tv_sec = 0;
		timerOff.it_value.tv_usec = 0;
		timerOff.it_value.tv_sec = 0;

		currentThread = mainThread;

		setitimer(ITIMER_PROF,&timer,NULL);
	}

	return 0;
};

/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield()
{
	// change current thread's state from Running to Ready
	// save context of this thread to its thread control block
	// switch from this thread's context to the scheduler's context
	setitimer(ITIMER_PROF, &timerOff, NULL);
	swapcontext(&(currentThread->context), &schedulerContext);
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr)
{
	// preserve the return value pointer if not NULL
	// deallocate any dynamic memory allocated when starting this thread
	setitimer(ITIMER_PROF,&timerOff,NULL);
	//put thread on finished stack
	finished_queue* finishedThread = malloc(sizeof(finished_queue));
	finishedThread->t_id = currentThread->t_id;
	finishedThread->value = value_ptr;
	if(finishedQueueHead == NULL)
	{
		finishedThread->next = NULL;
	}
	else{
		finishedThread->next=finishedQueueHead;
	}
	//set stack head to latest finished thread
	finishedQueueHead=finishedThread;
	finish = 1;
	//go back to scheduler
	setcontext(&schedulerContext);
	return;
};

/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr)
{
	// wait for a specific thread to terminate
	// deallocate any dynamic memory created by the joining thread
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		finished_queue* prev = NULL;
		finished_queue* curr = finishedQueueHead;

		while(curr!=NULL)
		{
			if(curr->t_id == thread)
			{
				if(curr->value != NULL)
				{
					value_ptr = &(curr->value);
				}

				if(prev == NULL)
				{
					finishedQueueHead = curr->next;
				}
				else{
					prev->next = curr->next;
				}
				free(curr);
				return 0;
			}
			prev = curr;
			curr = curr->next;
		}

		swapcontext(&(currentThread->context), &schedulerContext);
	}
	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	mutex = malloc(sizeof(mypthread_mutex_t));
	
	if(!mutex){return 1;}
	else{
	__atomic_clear(&mutex->lock_state,__ATOMIC_RELAXED);
	mutex->t_id = -1;
	mutex->blockedQueueHead = NULL;
	return 0;
	}
};

/* aquire a mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex)
{
	// use the built-in test-and-set atomic function to test the mutex
	// if the mutex is acquired successfully, return
	// if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
	if (__atomic_test_and_set(&mutex->lock_state,__ATOMIC_RELAXED)==1){
		setitimer(ITIMER_PROF, &timerOff, NULL);

		queue* newBlockedNode = malloc(sizeof(queue));
		newBlockedNode->threadControlBlock = currentThread;
		newBlockedNode->threadControlBlock->status = THREAD_SLEEPING;
		
		newBlockedNode->next = NULL;
		currentThread = NULL;

		if (mutex->blockedQueueHead == NULL){
			mutex->blockedQueueHead = newBlockedNode;
		}
		else{
			queue* crnt = mutex->blockedQueueHead;

			while (crnt->next != NULL){
				crnt = crnt->next;
			}

			crnt->next = newBlockedNode;
		}

		swapcontext(&(newBlockedNode->threadControlBlock->context), &schedulerContext);
	}
	
	mutex->lock_state = LOCKED;
	mutex->t_id = currentThread->t_id;
	
	return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex)
{
	// update the mutex's metadata to indicate it is unlocked
	// put the thread at the front of this mutex's blocked/waiting queue in to the run queue
	if (mutex->lock_state == LOCKED && mutex->t_id == currentThread->t_id)
	{
		__atomic_clear(&mutex->lock_state,__ATOMIC_RELAXED);
		mutex->t_id = UNTOUCHED;

		queue* crnt = mutex->blockedQueueHead;

		//take all blocked threads on this mutex and enqueue into runQueue
		while (crnt != NULL)
		{
			#ifndef MLFQ
				AddwithPi(crnt->threadControlBlock);
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
	free(mutex->blockedQueueHead);
	return 0;
};

/* scheduler */
static void schedule()
{
	// each priority a timer signal occurs your library should switch in to this context
	// be sure to check the SCHED definition to determine which scheduling algorithm you should run
	//   i.e. RR, PSJF or MLFQ
	#if defined(RR)
		sched_RR();
	#elif defined(MLFQ)
		sched_MLFQ();
	#else
		sched_PSJF();
	#endif
}

/* Round Robin scheduling algorithm */
static void sched_RR()
{
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(finish)
		{
			finish=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL)
		{
			currentThread->status=THREAD_UNINIT;
			currentThread->priority++;
			AddwithoutPi(currentThread);
		}

		currentThread = removeThread();
		if (currentThread == NULL){
        exit(EXIT_SUCCESS);
    }
		currentThread->status = THREAD_RUNNABLE;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	return;
}

/* Preemptive PSJF (STCF) scheduling algorithm */
static void sched_PSJF()
{
	// Your own implementation of PSJF (STCF)
	// (feel free to modify arguments and return types)
	while(1){
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(finish){
			finish=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL){
			currentThread->status = THREAD_UNINIT;
			currentThread->priority += 1;
			AddwithPi(currentThread);
		}

		currentThread = removeThread();
		currentThread->status = THREAD_RUNNABLE;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	}
	return;
}

/* Preemptive MLFQ scheduling algorithm */
/* Graduate Students Only */
static void sched_MLFQ() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(finish)
		{
			//free everyhting since thread is finish
			finish=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL)
		{
			//increase priority quantum 
			currentThread->status=THREAD_UNINIT;
			if(currentThread->priority!=5)
			{
				currentThread->priority+=1;
			}
			enqueueMLFQ(currentThread);
		}list_timer++;
		
		if (list_timer == 100){
			list_timer = 0;
			resetMLFQ();
		}

		//allow next thread to go
		currentThread = dequeueMLFQ();
		currentThread->status = THREAD_RUNNABLE;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	}

	return;
}


// Feel free to add any other functions you need
// YOUR CODE HERE
void signalHandler(int signum)
{
	swapcontext(&(currentThread->context),&schedulerContext);
}

void AddwithoutPi(tcb* threadBlock){
	queue* newRunNode = malloc(sizeof(queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	if (runQueueHead == NULL)
	{
		runQueueHead = newRunNode;
	}
	else
	{
		queue* crnt = runQueueHead;
		queue* prev = NULL;
		prev->next = newRunNode;
	}
}

void AddwithPi(tcb* threadBlock){
	queue* newRunNode = malloc(sizeof(queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	if (runQueueHead == NULL)
	{
		runQueueHead = newRunNode;
	}
	else
	{
		queue* crnt = runQueueHead;
		queue* prev = NULL;

		while (crnt != NULL)
		{
			if (crnt->threadControlBlock->priority > newRunNode->threadControlBlock->priority)
			{
				if (prev == NULL)
				{
					newRunNode->next = runQueueHead;
					runQueueHead = newRunNode;
				}
				else
				{
					prev->next = newRunNode;
					newRunNode->next = crnt;
				}
				return;
			}

			prev = crnt;
			crnt = crnt->next;
		}
		prev->next = newRunNode;
	}
}

tcb* removeThread()
{
	if (runQueueHead == NULL)
	{
		return NULL;
	}
	else
	{
		tcb* newThread = runQueueHead->threadControlBlock;
		runQueueHead = runQueueHead->next;
		return newThread;
	}
}

void enqueueMLFQ(tcb* threadBlock)
{
	queue* newRunNode = malloc(sizeof(queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	int index = newRunNode->threadControlBlock->priority;

	if (List[index] == NULL)
	{
		List[index] = newRunNode;
	}
	else{
		queue* curr = List[index];
		queue* prev = NULL;
		while(curr!=NULL)
		{
			prev=curr;
			curr=curr->next;
		}
		prev->next=newRunNode;
	}
}

tcb* dequeueMLFQ()
{
	for(int i = 0;i<6;i++)
	{
		if(List[i]!= NULL)
		{
			tcb* temp = List[i]->threadControlBlock;
			List[i] = List[i]->next;
			return temp;
		}
	}
	return NULL;
}

void resetMLFQ()
{
	for (int i = 1; i < 6; i++)
	{
		queue* crnt = List[i];

		if (crnt != NULL)
		{
			if (List[0] == NULL)
			{
				List[0] = List[i];
			}
			else
			{
				while (crnt->next != NULL)
				{
					crnt = crnt->next;
				}

				crnt->next = List[0];
				List[0] = List[i];
			}

			List[i] = NULL;
		}
	}
}