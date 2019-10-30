/*
 *
 * Simple preemption test 
 * the client program doesn't call uthread_yield() at all
 * It relies on preemption to switch the thread execution
 * 
 * The output will show that thread1, thread2 and main being printed sequentially and in arbitary orders
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uthread.h>
#include <limits.h>

int thread1(void *arg)
{
    int count = 0;
    while (1)
    {
        if (count == INT_MAX)
        {
            break;
        }
        if (count % 100000000 == 0)
        {
            printf("thread 1\n");
        }
        count++;
    }

    printf("\n=== thread 1 finished ===\n\n");

    return 0;
}

int thread2(void *arg)
{
    int count = 0;
    while (1)
    {
        if (count == INT_MAX)
        {
            break;
        }
        if (count % 100000000 == 0)
        {
            printf("thread 2\n");
        }
        count++;
    }

    uthread_join(1, NULL); /* wait for thread 3 to finish */

    printf("\n=== thread 2 finished ===\n\n");

    return 0;
}

int thread3(void *arg)
{
    int count = 0;
    while (1)
    {
        if (count == INT_MAX)
        {
            break;
        }
        if (count % 100000000 == 0)
        {
            printf("thread 3\n");
        }
        count++;
    }

    uthread_join(2, NULL); /* wait for thread 3 to finish */

    printf("\n=== thread 3 finished ===\n\n");

    return 0;
}

int main(void)
{
    uthread_create(thread1, NULL);
    uthread_create(thread2, NULL);
    uthread_create(thread3, NULL);

    int count = 0;
    while (1)
    {
        if (count == INT_MAX)
        {
            break;
        }
        if (count % 100000000 == 0)
        {
            printf("main thread\n");
        }
        count++;
    }

    uthread_join(3, NULL);

    printf("\n=== main finished ===\n\n");

    return 0;
}
