/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread2: thread 1 is already joined!
 * thread1
 * thread 3 joins thread 2...
 * thread2
 * thread 3 wakes up, thread 2 returned 2
 * main wakes up, thread1 returned 1
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void *arg)
{
	int retVal = -2;
	printf("thread 3 joins thread 2...\n");
	uthread_join(2, &retVal); /* thread 3 joins thread 2 */
	printf("thread 3 wakes up, thread 2 returned %d\n", retVal);

	return 3;
}

int thread2(void *arg)
{
	int ret = uthread_join(1, NULL); /* attempts to join thread 1, but failed */

	if (ret == -1)
	{
		printf("thread2: thread 1 is already joined!\n");
	}

	uthread_create(thread3, NULL);
	uthread_yield();
	printf("thread%d\n", uthread_self());

	return 2;
}

int thread1(void *arg)
{
	uthread_create(thread2, NULL);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	uthread_yield();
	return 1;
}

int main(void)
{
	int retVal = -2;
	uthread_join(uthread_create(thread1, NULL), &retVal);
	printf("main wakes up, thread1 returned %d\n", retVal);
	return 0;
}
