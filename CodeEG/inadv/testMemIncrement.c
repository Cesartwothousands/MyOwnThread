/*
Author: John-Austen Francisco
Date: 9 Oct 2022
Prec: Most common GCC libs
Post: Demonstration of inadvisable things in C
*/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>



#define STDIN 0
#define STDOUT 1
#define STDERR 2


/*
The inadvisably applied computer science series
 a.k.a. dumb tricks that shouldn't work

 Computer science is a contact sport. Get messy.

 Run the below on the iLabs, in particular any of the machines in the
  COMMAND cluster - doing this on different machines with different
  compilers, libraries, etc. may have different results.
*/


int main( int argc, char* argv[])
{
	// declare two strings
	char hello0[] = "hello?";
	char hello1[] = "HELLO!";
	
	// Look at indices of both:
	printf("%c\n", hello0[1]);
	printf("%c\n", hello1[1]);
	
	printf("%c\n", hello0[3]);
	printf("%c\n", hello1[3]);

	printf("%c\n", hello0[5]);
	printf("%c\n", hello1[5]);
	
	// What if we increment too far?
	printf("\n\n");
	printf("%c\n", hello0[10]);

	// Well, it didn't explode... hmm...
	int i = 0;
	while(i < 7)
	{
		printf("%c",hello0[6+i]);
		++i;
	}
	printf("\n\n");
	
	// Are we really seeing the other array?
	hello0[12] = '.';
	printf("%s\n", hello1);
	
	

	return 0;
}

