#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

typedef enum
{
	RUNNING,
	READY,
	ZOMBIE
} threadState_t; /* thread state enums */

typedef struct TCB
{
	uthread_t _tid;
	uthread_ctx_t _context;
	void *_stack;
	threadState_t _state;
} TCB; /* define thread control block */


typedef struct TCBController
{
	queue_t _queue; /* queue of waiting tds */
	TCB *_runningTd;
	uthread_t _count;

} TCBController;

static uthread_ctx_t mainCtx;

TCBController tcbController; /* maintains tcbs, this is a global var */

void uthread_yield(void)
{
	printf("%s\n", __func__);
	if (queue_length(tcbController._queue) == 0)
	{
		return; /* no other waiting thread avaible */
	}

printf("%s add\n", __func__);
	/* add current td to waiting queue */
	queue_enqueue(tcbController._queue, tcbController._runningTd);

	TCB *newRunningTd;
	TCB *oldRunningTd = tcbController._runningTd; /* make a copy */

printf("%s pop\n", __func__);
	/* obtain tcb of the next running td */
	queue_dequeue(tcbController._queue, (void **)&newRunningTd);

	tcbController._runningTd = newRunningTd;

printf("%s switch\n", __func__);
	/* context switch */
	uthread_ctx_switch(&(oldRunningTd->_context), &(newRunningTd->_context));
}

uthread_t uthread_self(void)
{
	printf("%s\n", __func__);
	return tcbController._runningTd->_tid;
}

int uthread_create(uthread_func_t func, void *arg)
{
	printf("%s\n", __func__);
	static bool isFirstInvocation = true;

	if (isFirstInvocation)
	{
		/* first invocation */
		tcbController._queue = queue_create();

		if (tcbController._queue == NULL)
		{
			return -1;
		}

		tcbController._count = 1;

		TCB *tcb = malloc(sizeof(TCB));
		tcb->_context = mainCtx;
		tcb->_tid = 0;
		tcb->_state = RUNNING;

		tcbController._runningTd = tcb; /* set main td as running td */

		isFirstInvocation = false;

	}

	/* subsequent calls to uthread_create */

	if (queue_length(tcbController._queue) == USHRT_MAX)
	{
		return -1;
	}

	void *stack = uthread_ctx_alloc_stack();

	if (stack == NULL)
	{
		return -1;
	}

	TCB *tcb = malloc(sizeof(TCB));
	tcb->_stack = stack;
	tcb->_tid = tcbController._count;
	tcbController._count++;
	tcb->_state = READY;

	if (uthread_ctx_init(&(tcb->_context), stack, func, arg) != 0)
	{
		return -1;
	}

	queue_enqueue(tcbController._queue, (void *)tcb);

	return tcb->_tid;
}

void uthread_exit(int retval)
{
	/* TODO Phase 2 */
}

int uthread_join(uthread_t tid, int *retval)
{
	printf("%s\n", __func__);
	while (true)
	{
		if (queue_length(tcbController._queue) == 0)
		{
			break;
		}
		uthread_yield();
	}

	return 0;
	/* TODO Phase 3 */
}
