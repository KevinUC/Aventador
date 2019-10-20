#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "queue.h"

typedef struct node
{
	void *_data;
	struct node *_prev;
	struct node *_next;
} node;

typedef struct queue
{
	node *_head; /* dummy head */
	node *_tail; /* dummy tail */
	int _length;
} queue; /* implemented as a double linked list */

queue_t queue_create(void)
{
	queue_t myQueue = malloc(sizeof(queue));

	if (myQueue == NULL)
	{
		return NULL;
	}

	myQueue->_length = 0;				   /* default queue length is 0 */
	myQueue->_head = malloc(sizeof(node)); /* init dummy head */

	if (myQueue->_head == NULL)
	{
		return NULL;
	}

	myQueue->_head->_prev = NULL;
	myQueue->_tail = malloc(sizeof(node)); /* init dummy tail */

	if (myQueue->_tail == NULL)
	{
		return NULL;
	}

	myQueue->_tail->_next = NULL;
	myQueue->_tail->_prev = myQueue->_head; /* tail -> head */
	myQueue->_head->_next = myQueue->_tail; /* head -> tail */

	return myQueue;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->_length > 0)
	{
		return -1;
	}

	free(queue->_head);
	free(queue->_tail);
	free(queue);

	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	node *newFirstNode = malloc(sizeof(node));

	if (newFirstNode == NULL)
	{
		return -1;
	}

	newFirstNode->_data = data;
	newFirstNode->_next = newFirstNode->_prev = NULL;

	node *oldFirstNode = queue->_head->_next;
	queue->_head->_next = newFirstNode;
	newFirstNode->_prev = queue->_head;
	newFirstNode->_next = oldFirstNode;
	oldFirstNode->_prev = newFirstNode;

	queue->_length++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || queue->_length == 0 || data == NULL)
	{
		return -1;
	}

	node *oldLastNode = queue->_tail->_prev;
	node *newLastNode = oldLastNode->_prev;

	queue->_tail->_prev = newLastNode;
	newLastNode->_next = queue->_tail;

	*data = oldLastNode->_data; /* assign data to oldest item */

	queue->_length--;

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	node *currentNode = queue->_tail->_prev; /* search from the oldest node */
	bool found = false;						 /* mark if data exists*/
	int ret = -1;							 /*return value of the function */

	while (currentNode != queue->_head)
	{
		if (currentNode->_data == data) /* data found */
		{
			found = true;
			ret = 0;
			currentNode->_prev->_next = currentNode->_next;
			currentNode->_next->_prev = currentNode->_prev;
			free(currentNode);
			break;
		}

		currentNode = currentNode->_prev;
	}

	if (found)
	{
		queue->_length--;
	}

	return ret;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL)
	{
		return -1;
	}

	node *currentNode = queue->_tail->_prev; /* iterate from the oldest node */

	while (currentNode != queue->_head)
	{
		if (func(currentNode->_data, arg) == 1)
		{
			if (data != NULL)
			{
				*data = currentNode->_data;
			}

			break;
		}

		currentNode = currentNode->_prev;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL)
	{
		return -1;
	}

	return queue->_length;
}
