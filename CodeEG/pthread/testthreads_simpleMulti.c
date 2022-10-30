/*
Author: John-Austen Francisco
Date: 9 Oct 2022
Prec: pthread library and common system libs
Post: demonstrate very basic use of multiple pthreads
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>

// A function we'll be calling as a thread
//  .. must be a void*/void*
void* threadFunc0(void* args)
{

	printf("I'm asynchronous function 0... Whahoo!\n");

	//the correct way to end a thread is with a pthread_exit
	pthread_exit(NULL);
}


// A function we'll be calling as a thread
//  .. must be a void*/void*
void* threadFunc1(void* args)
{

	printf("I'm asynchronous function 1... Whoo!\n");

	//the correct way to end a thread is with a pthread_exit
	pthread_exit(NULL);
}


// A function we'll be calling as a thread
//  .. must be a void*/void*
void* threadFunc2(void* args)
{

	printf("I'm asynchronous function 2... Wheee!\n");

	//the correct way to end a thread is with a pthread_exit
	pthread_exit(NULL);
}


int main( int argc, char* argv[])
{
	// build pthread handles
	pthread_t thread0;
	pthread_t thread1;
	pthread_t thread2;


	// build and init a pthread attribute struct
	// .. most of the time you only need the default attributes
	pthread_attr_t threadAttrs;
	pthread_attr_init(&threadAttrs);


	// create pthreads
	// .. so long as you do not need different options, you can use the same
	//      attrib struct for multiple threads
	pthread_create( &thread0, &threadAttrs, threadFunc0, NULL);
	pthread_create( &thread1, &threadAttrs, threadFunc1, NULL);
	pthread_create( &thread2, &threadAttrs, threadFunc2, NULL);

	// done creating threads, so destroy the attrib struct
	pthread_attr_destroy(&threadAttrs);

	// join so Process does not go away before the threads are done
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);


	return 0;
}

