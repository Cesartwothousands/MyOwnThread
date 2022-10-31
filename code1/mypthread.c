// File:	mypthread.c

// List all group members' names:
// iLab machine tested on:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

int TIMESEC = 1;
int MLFQtimer = 0;
run_queue* MLFQarray[6];
run_queue* runQueueHead = NULL;
finished_queue* finishedQueueHead = NULL;
int done = 0;
int t_idcounter = 0;
int firstTime = 1;
struct itimerval timer;
struct itimerval timerOff;
tcb* currentThread = NULL;
tcb* mainThread = NULL;
ucontext_t schedulerContext, mainContext;
void runner(void*(*function)(void*), void* arg);


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
	   // YOUR CODE HERE	
	   // create a Thread Control Block
	   // create and initialize the context of this thread
	   // allocate heap space for this thread's stack
	   // after everything is all set, push this thread into the ready queue
	if (firstTime){
		mainThread = myMalloc(sizeof(tcb));
		mainThread->t_id = t_idcounter++;
		mainThread->status = 0;
		mainThread->priority = 0;

		getcontext(&mainContext);

		mainThread->context = mainContext;
		
		#if defined(MLFQ)
			enqueueMLFQ(mainThread);
			dequeueMLFQ();
		#elif defined(PSJF)
			enqueueSTCF(mainThread);
			dequeueSTCF();
		#else
			enqueueRR(mainThread);
			dequeueRR();
		#endif

		getcontext(&schedulerContext);
		void* scheduleStack = myMalloc(SIGSTKSZ);
		schedulerContext.uc_link = NULL;
		schedulerContext.uc_stack.ss_sp = scheduleStack;
		schedulerContext.uc_stack.ss_size = SIGSTKSZ;
		schedulerContext.uc_stack.ss_flags = 0;
		makecontext(&schedulerContext, (void*)&schedule, 0);
	}

	//TCB
	tcb* newThread = myMalloc(sizeof(tcb));
	newThread->t_id = t_idcounter++;
	*thread = newThread->t_id;
	newThread->status = 0;
	newThread->priority = 0;
	//Thread Context
	ucontext_t current;

	if (getcontext(&current) == -1)
	{
		printf("Error, getcontext had an error\n");
		exit(1);
	}

	void* stack = myMalloc(SIGSTKSZ);
	current.uc_link = NULL;
	current.uc_stack.ss_sp = stack;
	current.uc_stack.ss_size = SIGSTKSZ;
	current.uc_stack.ss_flags = 0;
	makecontext(&current, (void*)&runner, 2, function, arg);
	newThread->context = current;
	//Runqueue

	#if defined(MLFQ)
		enqueueMLFQ(newThread);
	#elif defined(PSJF)
		enqueueSTCF(newThread);
	#else
		enqueueRR(newThread);
	#endif

	if (firstTime){
		firstTime = 0;

		struct sigaction signal;
		memset(&signal,0,sizeof(signal));
		signal.sa_handler = &signalHandler;
		sigaction(SIGPROF, &signal, NULL);

		timer.it_interval.tv_usec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = 5;
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
	// YOUR CODE HERE
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
	// YOUR CODE HERE

	// preserve the return value pointer if not NULL
	// deallocate any dynamic memory allocated when starting this thread
	setitimer(ITIMER_PROF,&timerOff,NULL);
	//put thread on finished stack
	finished_queue* finishedThread = myMalloc(sizeof(finished_queue));
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
	done = 1;
	//go back to scheduler
	setcontext(&schedulerContext);
	return;
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr)
{
	// YOUR CODE HERE

	// wait for a specific thread to terminate
	// deallocate any dynamic memory created by the joining thread
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		finished_queue* prev = NULL;
		finished_queue* curr = finishedQueueHead;
		//iterate through finished threads
		while(curr!=NULL)
		{
			//found the thread to join
			if(curr->t_id == thread)
			{
				if(curr->value != NULL)
				{
					value_ptr = &(curr->value);
				}
				//we are at the head
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
	// YOUR CODE HERE
	
	//initialize data structures for this mutex
	mutex = myMalloc(sizeof(mypthread_mutex_t));
	__atomic_clear(&mutex->locked,__ATOMIC_RELAXED);
	mutex->t_id = -1;
	mutex->blockedQueueHead = NULL;
	return 0;
};

/* aquire a mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex)
{
		// YOUR CODE HERE
	
		// use the built-in test-and-set atomic function to test the mutex
		// if the mutex is acquired successfully, return
		// if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
	if (__atomic_test_and_set(&mutex->locked,__ATOMIC_RELAXED)==1){
		setitimer(ITIMER_PROF, &timerOff, NULL);

		blocked_queue* newBlockedNode = myMalloc(sizeof(blocked_queue));
		newBlockedNode->threadControlBlock = currentThread;
		newBlockedNode->threadControlBlock->status = 2;
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
	
	mutex->locked = 1;
	mutex->t_id = currentThread->t_id;
		return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex)
{
	// YOUR CODE HERE	
	
	// update the mutex's metadata to indicate it is unlocked
	// put the thread at the front of this mutex's blocked/waiting queue in to the run queue
	if (mutex->locked == 1 && mutex->t_id == currentThread->t_id)
	{
		__atomic_clear(&mutex->locked,__ATOMIC_RELAXED);
		mutex->t_id = -1;

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
	// YOUR CODE HERE
	
	// deallocate dynamic memory allocated during mypthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule()
{
	// YOUR CODE HERE
	
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
	return;
}

/* Round Robin scheduling algorithm */
static void sched_RR()
{
	// YOUR CODE HERE
	
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(done)
		{
			//free everyhting since thread is done
			done=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL)
		{
			//increase priority quantum 
			currentThread->status=0;
			currentThread->priority+=1;
			enqueueRR(currentThread);
		}

		//allow next thread to go
		currentThread = dequeueRR();
		currentThread->status = 1;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	}
	return;
	
	return;
}

/* Preemptive PSJF (STCF) scheduling algorithm */
static void sched_PSJF()
{
	// YOUR CODE HERE

	// Your own implementation of PSJF (STCF)
	// (feel free to modify arguments and return types)
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(done)
		{
			//free everyhting since thread is done
			done=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL)
		{
			//increase priority quantum 
			currentThread->status=0;
			currentThread->priority+=1;
			enqueueSTCF(currentThread);
		}

		//allow next thread to go
		currentThread = dequeueSTCF();
		currentThread->status = 1;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	}
	return;
}

/* Preemptive MLFQ scheduling algorithm */
/* Graduate Students Only */
static void sched_MLFQ() {
	// YOUR CODE HERE
	
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	while(1)
	{
		setitimer(ITIMER_PROF,&timerOff,NULL);
		if(done)
		{
			//free everyhting since thread is done
			done=0;
			free((currentThread->context).uc_stack.ss_sp);
			free(currentThread);
			currentThread=NULL;
		}
		if(currentThread!=NULL)
		{
			//increase priority quantum 
			currentThread->status=0;
			if(currentThread->priority!=5)
			{
				currentThread->priority+=1;
			}
			enqueueMLFQ(currentThread);
		}

		MLFQtimer++;
		if (MLFQtimer == 100)
		{
			MLFQtimer = 0;
			resetMLFQ();
		}

		//allow next thread to go
		currentThread = dequeueMLFQ();
		currentThread->status = 1;
		setitimer(ITIMER_PROF,&timer,NULL);
		swapcontext(&schedulerContext, &(currentThread->context));
	}
	return;
}


// Feel free to add any other functions you need

// YOUR CODE HERE
void runner(void*(*function)(void*), void* arg)
{
	function(arg);
}

void signalHandler(int signum)
{
	swapcontext(&(currentThread->context),&schedulerContext);
}

void enqueueSTCF(tcb* threadBlock)
{
	run_queue* newRunNode = myMalloc(sizeof(run_queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	if (runQueueHead == NULL)
	{
		runQueueHead = newRunNode;
	}
	else
	{
		run_queue* crnt = runQueueHead;
		run_queue* prev = NULL;

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

tcb* dequeueSTCF()
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

void enqueueRR(tcb* threadBlock){
	run_queue* newRunNode = myMalloc(sizeof(run_queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	if (runQueueHead == NULL)
	{
		runQueueHead = newRunNode;
	}
	else
	{
		run_queue* crnt = runQueueHead;
		run_queue* prev = NULL;

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

tcb* dequeueRR()
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
	run_queue* newRunNode = myMalloc(sizeof(run_queue));
	newRunNode->threadControlBlock = threadBlock;
	newRunNode->next = NULL;

	int index = newRunNode->threadControlBlock->priority;

	if (MLFQarray[index] == NULL)
	{
		MLFQarray[index] = newRunNode;
	}
	else{
		run_queue* curr = MLFQarray[index];
		run_queue* prev = NULL;
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
	for(int i = 0;i<8;i++)
	{
		if(MLFQarray[i]!= NULL)
		{
			tcb* temp = MLFQarray[i]->threadControlBlock;
			MLFQarray[i] = MLFQarray[i]->next;
			return temp;
		}
	}
	return NULL;
}

void resetMLFQ()
{
	for (int i = 1; i < 8; i++)
	{
		run_queue* crnt = MLFQarray[i];

		if (crnt != NULL)
		{
			if (MLFQarray[0] == NULL)
			{
				MLFQarray[0] = MLFQarray[i];
			}
			else
			{
				while (crnt->next != NULL)
				{
					crnt = crnt->next;
				}

				crnt->next = MLFQarray[0];
				MLFQarray[0] = MLFQarray[i];
			}

			MLFQarray[i] = NULL;
		}
	}
}

void* myMalloc(int size)
{
	void* temp = calloc(1, size);

	if (temp == NULL)
	{
		printf("Fatal Error, malloc has returned null");
		exit(1);
	}

	return temp;
}