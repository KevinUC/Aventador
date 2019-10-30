#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../libuthread/queue.h"

/* This unit test is referenced from page 6 of the assignment prompt (ECS-150-Project 2) */
void testCreate(void)
{
    queue_t q1 = NULL;
    queue_t q2 = NULL;

    q1 = queue_create();
    q2 = queue_create();

    assert(q1 != NULL);
    assert(q2 != NULL);
}

void testDestroy(void)
{
    queue_t q = queue_create();

    assert(queue_destroy(q) == 0);
}

void testDestroyNull(void)
{
    queue_t q = NULL;

    assert(queue_destroy(q) == -1);
}

void testDestroyNonEmpty(void)
{
    queue_t q = queue_create();

    int i = 1;

    queue_enqueue(q, &i);

    assert(queue_destroy(q) == -1);
}

void testEnqueueErrorHandle(void)
{
    queue_t q1 = queue_create();
    queue_t q2 = NULL;
    int i = 1;

    assert(queue_enqueue(q1, NULL) == -1);
    assert(queue_enqueue(q2, &i) == -1);
}

void testDequeueErrorHandle(void)
{
    queue_t q1 = queue_create();
    queue_t q2 = NULL;
    int *i;

    assert(queue_dequeue(q1, NULL) == -1);
    assert(queue_dequeue(q2, (void **)&i) == -1);
    assert(queue_dequeue(q1, (void **)&i) == -1); /* queue is empty */
}

void testEnqueueAndDequeue(void)
{
    queue_t q;
    q = queue_create();

    int numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    int *ptr;

    for (int i = 0; i <= 16; i++)
    {
        assert(queue_enqueue(q, &numbers[i]) == 0);
    }

    assert(queue_length(q) == 17);

    for (int i = 0; i <= 15; i++)
    {
        assert(queue_dequeue(q, (void **)&ptr) == 0);
        assert(ptr == &numbers[i]);
    }

    assert(queue_dequeue(q, NULL) == -1);
    assert(queue_dequeue(q, (void **)&ptr) == 0);
    assert(ptr == &numbers[16]);
    assert(queue_length(q) == 0);
}

void testDeleteErrorHandle(void)
{
    queue_t q1 = queue_create();
    queue_t q2 = NULL;

    int *data1 = NULL;
    int i = 0;

    assert(queue_delete(q2, &i) == -1);
    assert(queue_delete(q1, data1) == -1);
    assert(queue_delete(q1, &i) == -1); /* data is not found in queue */
}

void testDelete(void)
{
    queue_t q;
    q = queue_create();

    int numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 200};
    char chs[] = {'a', 'b', 'c', 'd', 'e', 'f'};
    char strs[][10] = {"hello", "how", "are", "you"};

    for (int i = 0; i <= 4; i++)
    {
        queue_enqueue(q, &numbers[i]);
    }

    for (int i = 0; i <= 5; i++)
    {
        queue_enqueue(q, &chs[i]);
    }

    for (int i = 5; i <= 9; i++)
    {
        queue_enqueue(q, &numbers[i]);
    }

    assert(queue_length(q) == 16);
    assert(queue_delete(q, &numbers[4]) == 0);  /* delete numbers[4] */
    assert(queue_delete(q, &numbers[4]) == -1); /* numbers[4] is removed */
    assert(queue_delete(q, &numbers[10]) == -1);

    assert(queue_delete(q, &chs[0]) == 0);  /* delete chs[0] */
    assert(queue_delete(q, &chs[0]) == -1); /* chs[0] is removed */

    assert(queue_length(q) == 14);

    for (int i = 0; i <= 3; i++)
    {
        queue_enqueue(q, &strs[i]);
    }

    for (int i = 0; i <= 3; i++)
    {
        assert(queue_delete(q, &strs[i]) == 0);
    }

    for (int i = 0; i <= 5; i++)
    {
        queue_delete(q, &chs[i]);
    }

    for (int i = 0; i <= 9; i++)
    {
        queue_delete(q, &numbers[i]);
    }

    queue_enqueue(q, &strs[2]);
    assert(queue_delete(q, &strs[2]) == 0);

    assert(queue_length(q) == 0);
}

void testIterateErrorHandle(void)
{
    assert(queue_iterate(NULL, NULL, NULL, NULL) == -1);
}

/* This callback function is modified from page 7 of the assignment prompt (ECS-150-Project 2) 
   it takes the absolute value of the given value 
*/

int absoluteValue(void *data, void *arg)
{
    int *num = (int *)data;

    if (*num < 0)
    {
        *num *= -1;
    }

    return 0;
}

/* This callback function is modified from page 7 of the assignment prompt (ECS-150-Project 2) 
   it finds the first value larger than the given threshold 
*/

int findNum(void *data, void *arg)
{
    int *num = (int *)data;
    int threshold = (int)(long)arg;

    if (*num > threshold)
        return 1;

    return 0;
}

/* This function is partially referenced from page 7-8 of the assignment prompt (ECS-150-Project 2) 
   it takes the absolute value of the given value 
*/

void testIterate(void)
{
    queue_t q = queue_create();

    int numbers[] = {-1, -1000, -3, 4, 5, 6, 7, 8, 9, 10, -200};
    int *ptr = NULL;

    for (int i = 0; i <= 10; i++)
    {
        queue_enqueue(q, &numbers[i]);
    }

    queue_iterate(q, absoluteValue, NULL, NULL); /* take absolute value of each element of the array */

    assert(numbers[2] == 3);

    queue_iterate(q, findNum, (void *)199, (void **)&ptr); /* find first number larger than 199 */

    assert(ptr != NULL);
    assert(ptr == &numbers[1]);
    assert(*ptr == 1000);
}

void testQueueLengthErrorHandle(void)
{
    assert(queue_length(NULL) == -1);
}

void testQueueLength(void)
{
    int list[] = {1, 2, 3};
    queue_t q = queue_create();

    queue_enqueue(q, &list[0]);
    queue_enqueue(q, &list[1]);

    assert(queue_length(q) == 2);
}

int main(int argc, char *argv[])
{
    /* run all the unit tests */

    testCreate();
    testDestroy();
    testDestroyNull();
    testDestroyNonEmpty();
    testQueueLengthErrorHandle();
    testQueueLength();
    testEnqueueErrorHandle();
    testDequeueErrorHandle();
    testEnqueueAndDequeue();
    testDeleteErrorHandle();
    testDelete();
    testIterateErrorHandle();
    testIterate();
}