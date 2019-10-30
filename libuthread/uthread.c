#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define initQueue(x)        \
	{                       \
		x = queue_create(); \
		if (x == NULL)      \
		{                   \
			return -1;      \
		}                   \
	}

typedef struct TCB
{
	uthread_t _tid;
	uthread_ctx_t _context;
	void *_stack;
	int _retval;
} TCB; /* define thread control block */

typedef struct TCBController
{
	queue_t _readyQueue;   /* queue of ready tds */
	queue_t _blockedQueue; /* queue of blocked tds */
	queue_t _zombieQueue;  /* queue of zombie tds */

	/* thread _JoinedBy[tid] waits for thread tid to finish */
	uthread_t _JoinedBy[USHRT_MAX];

	bool _isTidValid[USHRT_MAX]; /* mark if thread still exists (running, 	
								     blocked or ready ) */

	TCB *_runningTd;  /* current running thread */
	uthread_t _count; /* total number of threads created so far, including dead 
						 ones */

} TCBController;

static uthread_ctx_t mainCtx;

TCBController tcbController; /* maintains tcbs, this is a global var */

void switchToFirstReadyTd()
{
	TCB *newRunningTd;
	TCB *oldRunningTd = tcbController._runningTd; /* make a copy */

	/* obtain tcb of the next running td */
	queue_dequeue(tcbController._readyQueue, (void **)&newRunningTd);

	tcbController._runningTd = newRunningTd;

	/* context switch */
	uthread_ctx_switch(&(oldRunningTd->_context), &(newRunningTd->_context));
}

void uthread_yield(void)
{
	preempt_disable();

	if (queue_length(tcbController._readyQueue) == 0)
	{
		preempt_enable();
		return; /* no other waiting thread avalible */
	}

	/* add current td to waiting queue */
	queue_enqueue(tcbController._readyQueue, tcbController._runningTd);

	switchToFirstReadyTd();
}

uthread_t uthread_self(void)
{
	preempt_disable();
	uthread_t tid = tcbController._runningTd->_tid;
	preempt_enable();

	return tid;
}

int uthread_init()
{
	/* init tcb */

	TCB *tcb = malloc(sizeof(TCB));

	if (tcb == NULL)
	{
		return -1;
	}

	tcb->_context = mainCtx;
	tcb->_tid = 0;

	/* init tcbController */
	initQueue(tcbController._readyQueue);
	initQueue(tcbController._blockedQueue);
	initQueue(tcbController._zombieQueue);

	memset(tcbController._JoinedBy, USHRT_MAX, sizeof(tcbController._JoinedBy));
	memset(tcbController._isTidValid, false, sizeof(tcbController._isTidValid));

	tcbController._count = 1;
	tcbController._isTidValid[0] = true; /* we just registered main thread */
	tcbController._runningTd = tcb;		 /* set main td as running td */

	return 0;
}

int uthread_create(uthread_func_t func, void *arg)
{

	static bool isFirstInvocation = true;

	/* initialize if called the first time */

	if (isFirstInvocation)
	{
		if (uthread_init() != 0)
		{
			return -1; /* uthread_init failed */
		}

		isFirstInvocation = false;
		preempt_start(); /* enable preemption */
	}

	preempt_disable();

	if (tcbController._count == USHRT_MAX)
	{
		preempt_enable();
		return -1; /* TID value overflow */
	}

	/* create new thread */

	void *stack = uthread_ctx_alloc_stack();

	if (stack == NULL)
	{
		preempt_enable();
		return -1;
	}

	TCB *tcb = malloc(sizeof(TCB));
	tcb->_stack = stack;
	tcb->_tid = tcbController._count;
	tcb->_retval = -100;

	tcbController._count++;
	tcbController._isTidValid[tcb->_tid] = true;

	if (uthread_ctx_init(&(tcb->_context), stack, func, arg) != 0)
	{
		preempt_enable();
		return -1;
	}

	queue_enqueue(tcbController._readyQueue, (void *)tcb);

	preempt_enable();

	return tcb->_tid;
}

/* Callback function that finds the tcb */
int findThread(void *data, void *arg)
{
	uthread_t tid = (uthread_t)(long)arg;
	TCB *tcb = (TCB *)data;

	if (tcb->_tid == tid)
	{
		return 1;
	}

	return 0;
}

void uthread_exit(int retval)
{
	preempt_disable();

	/* wake up joined thread if there is any */

	int joinerTid = tcbController._JoinedBy[uthread_self()];

	if (joinerTid != USHRT_MAX)
	{
		/* remove joiner thread from blocked queue */

		TCB *joinerTcb = NULL;
		queue_iterate(tcbController._blockedQueue, findThread, (void *)(long)joinerTid, (void **)&joinerTcb);

		if (joinerTcb != NULL && joinerTcb->_tid == joinerTid)
		{
			queue_delete(tcbController._blockedQueue, joinerTcb);
		}

		/* add joiner thread to ready queue */

		queue_enqueue(tcbController._readyQueue, joinerTcb);
	}

	/* put running thread into zombie queue */

	tcbController._runningTd->_retval = retval; /* store retval into tcb */
	queue_enqueue(tcbController._zombieQueue, tcbController._runningTd);

	/* yield control to next avalible thread */

	switchToFirstReadyTd();
}

bool isTidValid(uthread_t tid)
{
	return tid <= tcbController._count - 1 && tid != 0 && tid != uthread_self() && tcbController._isTidValid[tid] && tcbController._JoinedBy[tid] == USHRT_MAX;
}

int collectJoinedThread(uthread_t tid, int *retval)
{
	TCB *tcb = NULL;
	queue_iterate(tcbController._zombieQueue, findThread, (void *)(long)tid, (void **)&tcb);

	if (tcb != NULL && tcb->_tid == tid)
	{
		/* joining tid is already dead (in zombie state) 
		 * clean up the dead thread 
		 */
		if (retval != NULL)
		{
			*retval = tcb->_retval; /* assign return status to retval */
		}

		queue_delete(tcbController._zombieQueue, tcb);
		uthread_ctx_destroy_stack(tcb->_stack);
		free(tcb);

		tcbController._JoinedBy[tid] = USHRT_MAX;
		tcbController._isTidValid[tid] = false;

		return 0;
	}

	return -1; /* tid not found */
}

int uthread_join(uthread_t tid, int *retval)
{
	/* check if tid is valid for joining */
	preempt_disable();

	if (!isTidValid(tid))
	{
		preempt_enable();
		return -1;
	}

	tcbController._JoinedBy[tid] = uthread_self();

	/* check if the joining tid is already dead */

	if (collectJoinedThread(tid, retval) == 0)
	{
		preempt_enable();
		return 0;
	}

	/* block current thread and release control */

	queue_enqueue(tcbController._blockedQueue, (void *)tcbController._runningTd);

	switchToFirstReadyTd();

	/* wake up from here, start cleaning joined thread */

	preempt_disable();
	collectJoinedThread(tid, retval);
	preempt_enable();

	return 0;
}
