#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../libuthread/queue.h"

/* This unit test is directly copied from the assignment prompt */
void test_create(void)
{
    queue_t q;
    q = queue_create();
    assert(q != NULL);
    printf("test_create OK ...\n");
}

/* This unit test is referenced from the assignment prompt */
void test_queue_simple(void)
{
    queue_t q;
    int data = 3, *ptr;
    q = queue_create();
    assert(queue_length(q) == 0);

    queue_enqueue(q, &data);
    assert(queue_length(q) == 1);

    queue_dequeue(q, (void **)&ptr);
    assert(ptr == &data);
    assert(queue_length(q) == 0);

    assert(queue_destroy(q) == 0);
    printf("test_queue_simple OK ...\n");
}

void test_queue_delete(void)
{
    queue_t q;
    q = queue_create();

    int numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 200};
    char chs[] = {'a', 'b', 'c', 'd', 'e', 'f'};
    char strs[][10] = {"hello", "how", "are", "you"};

    assert(queue_delete(q, NULL) == -1);       /* delete NULL */
    assert(queue_delete(NULL, &chs[2]) == -1); /* queue is NULL */

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

    printf("test_queue_delete OK ...\n");
}

void test_enqueue_and_dequeue(void)
{
    queue_t q;
    q = queue_create();

    int numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    int *ptr;

    assert(queue_enqueue(q, NULL) == -1);
    assert(queue_enqueue(NULL, &numbers[1]) == -1);

    assert(queue_dequeue(q, NULL) == -1);
    assert(queue_dequeue(NULL, (void **)&ptr) == -1);
    assert(queue_dequeue(q, (void **)&ptr) == -1);

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

    printf("test_enqueue_and_dequeue OK ...\n");
}

/* Callback function that increments items by a certain value */
static int inc_item(void *data, void *arg)
{
    int *a = (int *)data;
    int inc = (int)(long)arg;
    *a += inc;
    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(void *data, void *arg)
{
    int *a = (int *)data;
    int match = (int)(long)arg;
    if (*a == match)
        return 1;
    return 0;
}

static int print_int_item(void *data, void *arg)
{
    int *a = (int *)data;
    printf("%d\n", *a);
    return 0;
}

void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int i;
    int *ptr;
    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);
    /* Add value '1' to every item of the queue */
    queue_iterate(q, inc_item, (void *)1, NULL);
    assert(data[0] == 2);
    /* Find and get the item which is equal to value '5' */
    ptr = NULL;
    queue_iterate(q, find_item, (void *)5, (void **)&ptr);
    assert(ptr != NULL);
    assert(*ptr == 5);
    assert(ptr == &data[3]);

    printf("test_iterator OK ...\n");
}

int main(int argc, char *argv[])
{
    test_create();
    test_queue_simple();
    test_enqueue_and_dequeue();
    test_queue_delete();
    test_iterator();
}