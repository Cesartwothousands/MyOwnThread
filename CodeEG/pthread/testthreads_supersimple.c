/*
Author: John-Austen Francisco
Date: 9 Oct 2022
Prec: pthread library and common system libs
Post: demonstrate very basic use of pthreads
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>

// The function we'll be calling as a thread
//  .. must be a void*/void*
void* threadFunc(void* args)
{

	printf("I'm an asynchronous function ... Whahoo!\n");

	//the correct way to end a thread is with a pthread_exit
	pthread_exit(NULL);
}



int main( int argc, char* argv[])
{
	// build a pthread handle
	pthread_t thread0;

	// build and init a pthread attribute struct
	// .. the default attributes are, most times, what you want
	pthread_attr_t threadAttrs;
	pthread_attr_init(&threadAttrs);


	// create a pthread
	pthread_create( &thread0, &threadAttrs, threadFunc, NULL);

	// done creating threads, so destroy the attrib struct
	pthread_attr_destroy(&threadAttrs);

	// join so Process does not go away before the thread is done
	pthread_join(thread0, NULL);


	return 0;
}

