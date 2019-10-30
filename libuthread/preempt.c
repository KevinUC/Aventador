#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

void preempt_disable(void)
{
	/* external reference: https://www.linuxprogrammingblog.com/code-examples/blocking-signals-with-sigprocmask*/
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
}

void preempt_enable(void)
{
	/* external reference: https://www.linuxprogrammingblog.com/code-examples/blocking-signals-with-sigprocmask*/
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void sigvtalrmHandler(int sigNum)
{
	uthread_yield();
}

void preempt_start(void)
{
	/* initialize signal action struct */
	struct sigaction _sigaction;

	memset(&_sigaction, 0, sizeof(_sigaction)); /* we didn't use sa_mask and sa_flags */
	_sigaction.sa_handler = &sigvtalrmHandler;

	if (sigaction(SIGVTALRM, &_sigaction, NULL) != 0)
	{
		exit(1);
	}

	/* initialize timer interval */

	struct itimerval _itimerval;

	_itimerval.it_interval.tv_sec = 0;
	_itimerval.it_interval.tv_usec = (1000 / HZ) * 1000;

	_itimerval.it_value.tv_sec = 0;
	_itimerval.it_value.tv_usec = (1000 / HZ) * 1000;

	if (setitimer(ITIMER_VIRTUAL, &_itimerval, NULL) != 0)
	{
		exit(1);
	}
}
