# Abstract:
As we've learned, a thread is a another stack in the same Process. Since a stack is an organization on memory, there is no reason why you can not create your own threads on the heap.

In this project you'll create a user-level thread library using the pthread library as a template. Since you will be building the threads, you will have to schedule them to run and also implement mutual exclusion in the form of mutexes. You will learn about some of the same scheduling issues faced by operating system designers when implementing Processes and kernel-level threads.

 

# Introduction:
As a user can you do not have the ability to turn off interrupts or determine when signals occur. You can not run code simultaneously, so your threads will be 'logical'. Your Process will have to split its runtime between multiple stacks. You will need to write code to allocate some heap space and give it a thread format and some code to act as a scheduler. Your scheduler will need to run at regular intervals and determine which thread-typed struct to run.

Being in charge of scheduling means you are also in charge of mutual exclusion. Your scheduler will need to handle swapping out threads that need to block due to mutual exclusion. If a thread is not running, it is certainly unable to access shared memory.

Decompress code.tar.gz on an iLab machine. It will contain a stubbed-out source file and header as well as a benchmarking harness for your use. You can find code.tar.gz in the Files section of Canvas under A0.


You will need to implement the following:

int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
\- this function creates a new 'mypthread'
\- you may ignore the attribute struct and presume it will be passed a NULL ptr
\- your return values should mirror pthread_create

void mypthread_yield();
\- functionally a manual call to your scheduler

void mypthread_exit(void *value_ptr);
\- a library call to explicitly end the calling thread
\- if the return pointer is not NULL, you will need to save the value to be returned

int mypthread_join(mypthread_t thread, void **value_ptr);
\- pauses the calling thread until the thread passed as a parameter exit()s
\- if value_ptr is not NULL, you will need to import the return value
\- your return values should mirror pthread_join


int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
\- this function sets up a new 'mutex'
\- you may ignore the attribute struct and presume it will be passed a NULL ptr
\- your return values should mirror pthread_mutex_init

int mypthread_mutex_lock(mypthread_mutex_t *mutex);
\- on run the calling thread either obtains the mutex, if free, or is deschedued if not until the mutex is freed
\- your return values should mirror pthread_mutex_lock

int mypthread_mutex_unlock(mypthread_mutex_t *mutex);
\- on run the calling thread unblocks the mutex, allowing a thread waiting for it to be scheduled to run
\- your return values should mirror pthread_mutex_unlock

int mypthread_mutex_destroy(mypthread_mutex_t *mutex);
\- deallocate and clean up a mutex
\- be sure the mutex is ulocked before allowing it to be destroy()ed
\- your return values should mirror pthread_mutex_destroy

 

# Methodology:

## Thread Creation:
You should investigate the ucontext library for creation and swapping of your threads. You should be sure to set the return pointer to a default context that only executes a mypthread_exit(), in case a careless user attempts to return out of a thread.

You should build a thread control block, akin to to Process control block, to denote a thread's metadata (i.e. thread ID) and current run state.

You will likely want to record a thread's state, like READY to run, BLOCKED on a mutex or currently RUNNING. Feel free to create or add your own run states.


## Scheduler Invocation:
You will need to interrupt your current thread and run your scheduler every so often. You should investigate setitimer() to cause a single signal at a set duration or to periodically raise a signal at regular intervals and signal() to catch it in order to run your scheduler code. While you are in the signal handler none of your other code is running, so it is safe to update any special state variables.

Take care if you set periodic signals since they can occur at any time, like while you are in the middle of a mutex or exit call. You should be careful to disable the timer while you are in the thread library and to include failsafe guard variables so that your scheduler does not attempt to run while you are running library code.


## Scheduling:

Be sure to run your scheduler whenever a thread blocks on a mutex, exit()s, or yield()s. Be careful not to schedule a thread to run that is currently blocking on a mutex.

Round robin:
You should first implement a fair share, round robin scheduler. Choose a quantum of time and run all threads issued for one quantum at a time, rotating through the list in call order.

Pick a time quantum that optimizes the runtime of the benchmarking code.


## SJF:
Your next scheduler should attempt to approximate shortest job first. You do not know the entire runtime of any thread, so approximate it by running any newly-created thread immediately for one quantum (pre-emptive) and incrementing a quantum count in the thread's TCB. Then schedule any thread to run that has run the least. In order to break ties, pick the thread that has been waiting the longest.

Pick a time quantum that optimizes the runtime of the benchmarking code.


## MLFQ:
Your last scheduler should implement a multilevel feedback queue. Modify your SJF scheduler to take in to account the amount of time a thread has been waiting and the amount of runtime it has consumed in order to calculate a priority value. You should swap in the thread with the highest priority on each scheduling decision.

Be sure to increase the priority of waiting jobs so that they eventually get runtime and are not starved out. Be sure to decrease the priority of jobs that have consumed runtime without exit()ing to clear runtime for new jobs. Balancing the rate that waiting jobs increase in priority and running jobs decrease in priority is the crux of tuning your scheduler.

Your MLFQ should be optimistic and start new threads at highest priority.


## Benchmarking:
Along with the benchmarking harness provided, create five more benchmarks and add entries to compile and run them to the project Makefiles. You may want to look up various CPU and IO benchmarking suites like SPEC or DHRYSTONE. Be sure to document why you chose the benchmarking parameters that you did. The fact that a given benchmark is part of an established protocol is not a reason.

 

# Results:

Be sure to fully comment your code.

Report:
Prepare a report.pdf with your partners' NetIDs.

Document your design and implementation details, including but not limited to your quantum chosen, tcb values and mutex metadata. Any additional data values, scheduling stages or data structures should be detailed in your report.You should benchmark at least three different quanta for your schedulers.

Be sure to detail the priority levels of your MLFQ and under what conditions you increase and decrease priority.

Profile both your code and the pthread library using the included benchmarks and compare their performance in your report.


# Submission:
Prepare and submit a A0.tar.gz that holds:
mypthread.h
mypthread.c
Makefile
report.pdf
Any other source files needed that you wrote.

Be sure that all group members submit. If you do not submit we can not assign your user a grade in Canvas.