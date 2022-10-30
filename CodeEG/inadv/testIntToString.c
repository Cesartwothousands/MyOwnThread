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
	// declare an int
	int val = 2189672;
	
	// build a pointer to point at the intetger created above
	int* ptr = &val;
	
	// demonstrate the the int name (i.e. 'ptr') holds
	//   the address of the start of four bytes of space
	//   and is an integer
	printf("%d\n", sizeof(*ptr));
	printf("%d\n", *ptr);

	// since all memory addresses are the same size,
	//   you can freely cast their type. Memory address type
	//   only affects what you dereference, not the address itself
	char* text = (char*)ptr;

	// in C chars are one byte long, so 'text' must
	//  point to at least four bytes of memory you are allowed to use
	//   .. recall all string are null-terminated char arrays
	//
	// on the iLabs, integers are four bytes long, so four contigusous bytes
	//  in memory can store either a four char array or a int, but can it hold both?
	printf("%s\n", text);
	printf("%d\n", *ptr);




	return 0;
}

