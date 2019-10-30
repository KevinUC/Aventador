/*
 * Simple hello world test
 *
 * Tests the creation of a single thread and its successful return.
 * 
 * The output will look like:
 * 
 * Hello World!
 * hello() returned 0
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int hello(void *arg)
{
	printf("Hello world!\n");
	return 0;
}

int main(void)
{
	uthread_t tid;
	int retVal = -1;

	tid = uthread_create(hello, NULL);
	uthread_join(tid, &retVal);
	printf("hello() returned %d\n", retVal);

	return 0;
}
