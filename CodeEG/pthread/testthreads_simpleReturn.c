/*
Author: John-Austen Francisco
Date: 9 Oct 2022
Prec: pthread library and common system libs
Post: demonstrate basic use of pthreads with return values
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>

// The function we'll be calling as a thread
//  .. must be a void*/void*
void* threadName(void* args)
{
	if(args == NULL)
	{
		printf("Set the thread id!\n");
		pthread_exit(NULL);
	}

	// cast and grab param
	int id = *((int*)args);

	// use it to demonstrate we have it
	printf("I'm function #%d! Whahooo!\n", id);

	// generate a return val
	// .. be sure to put it on the heap, so it doesn't go away when
	//      this thread exits
	int* result = (int*)malloc(sizeof(int));
	*result = 42;

	// rather than return with NULL, we have something to return
	//  .. cast retval to void* since we may be returning a pointer to anything
	pthread_exit((void*)result);
}



int main( int argc, char* argv[])
{
	// build thread handle
	pthread_t thread0;

	// build and init thread attibutes to defaults
	pthread_attr_t threadAttrs;
	pthread_attr_init(&threadAttrs);

	// make some heap space for the param to pass to
	//   the thread
	int* tid0 = (int*)malloc(sizeof(int));
	*tid0 = 0;

	// create the thread, passing the parameter pointer as a void*
	pthread_create( &thread0, &threadAttrs, threadName, (void*)tid0);
	pthread_attr_destroy(&threadAttrs);

	// prepare a retval pointer
	// .. it must be a void** later
	int** resultValue = (int**)malloc(sizeof(int*));

	// join our thread and set the retval pointer as non-NULL, since we care about it
	pthread_join(thread0, (void**)resultValue);

	// once join returns, our retval pointer should be pointing at 
	//  the address of our retval, so double dereference it
	//  .. we treat it as an int because we know it is an int because we wrote the code
	printf("Thread result: %d\n", **resultValue);

	// free all the memory allocated INCLUDING the memory allocated in thread
	//  for the retval - that's our responsibility, too!
	free(tid0);
	free(*resultValue);
	free(resultValue);

	return 0;
}

