//printf("%s\n", __func__);

typedef enum
{
	RUNNING,
	READY,
	ZOMBIE
} threadState_t; /* thread state enums */

//printf("thread %d is calling exit(%d)\n", uthread_self(), retval);