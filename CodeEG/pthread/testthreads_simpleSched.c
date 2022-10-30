/*
Author: John-Austen Francisco
Date: 9 Oct 2022
Prec: pthread library, math library and common system libs
Post: demonstrate basic use of pthreads with some delay to see scheduling happen
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>



// The function we'll be running as a thread
void*  printStuff(void* args)
{
	// initialize locals
	int i = 0;
	double j = 0.0;
	double result = 0.0;

	// init rand
	srand((unsigned int)time(NULL));
	// init local from param
	int myNum = *( (int *)args );

	// loop five times, printing ID and iteration
	//  .. in between, run some heavy trig functions to cause some random delay,
	//      this should hopefully show threads being swapped between as they
	//      suck up runtime
	while( i <= 5 )
	{
		printf("Thread #%d, iter #%d\n", myNum, i);
		++i;
		j = (double)(rand()%100);

		// wasting random trig time...
		while( j > 0 )
		{
			result = result + sin(j) - tan(j);
			--j;
		}
	}

	// prepare retval
	//  .. we don't really care about it, but the compiler is in most cases
	//      clever enough to realize that if you never need a value, there is
	//      no reason to compute it, so we need to pass this back and print it out.
	//      else it will be optimized away
	double* retvalloc = (double*)malloc(sizeof(double));
	*retvalloc = 100*result;

	// exit is the only correct way to leave threads (.. don't return!)
	pthread_exit((void*)retvalloc);
}



int main( int argc, char* argv[])
{
	// build thread handles for pthread_create
	pthread_t thread0;
	pthread_t thread1;

	// build a pthread attribute struct and initialize it
	//  .. if you don't need different attribs, you can use the same
	//      struct for multiple threads
	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);

	//create some arguments to pass to the threads
	// ... note they are malloced because they must be on the heap,
	//     so that they are visible to the threads
	int * threadArgs0 = (int*)malloc(sizeof(int));
	int * threadArgs1 = (int*)malloc(sizeof(int));
	*threadArgs0 = 0;
	*threadArgs1 = 1;
	

	// build the pthreads
	pthread_create(&thread0, &threadAttr, printStuff, (void*)threadArgs0);
	pthread_create(&thread1, &threadAttr, printStuff, (void*)threadArgs1);
	
	// destroy the pthread attribute structs, we're done creating the threads,
	//   we don't need them anymore
	pthread_attr_destroy(&threadAttr);

	
	// build thread status variables for pthread_exit to use
	void** threadStatus0 = (void**)malloc(sizeof(void*));
	void** threadStatus1 = (void**)malloc(sizeof(void*));
	
	// wait for the threads to finish .. make the threadStatus variables point to 
	//    the value of pthread_exit() called in each
printf("START WAITING\n");
	pthread_join( thread0, threadStatus0);
	pthread_join( thread1, threadStatus1);
printf("DONE WAITING\n");

	
	printf("%lf\n", **((double**)threadStatus0) );
	printf("%lf\n", **((double**)threadStatus1) );


	// when freeing it can be a good idea to delay a little between
	//   freeing both parts of a double indirection so you don't come up
	//    against timing issues
	free(*threadStatus0);
	free(*threadStatus1);
	free(threadArgs0);
	free(threadArgs1);
	free(threadStatus0);
	free(threadStatus1);


	return 0;
}

