all: libuthread.a

libuthread.a: queue.o uthread.o context.o preempt.o
	ar rcs libuthread.a queue.o uthread.o context.o preempt.o

queue.o: queue.c queue.h
	gcc -Wall -Werror -c -o queue.o queue.c

uthread.o: uthread.c uthread.h preempt.h context.h queue.h
	gcc -Wall -Werror -c -o uthread.o uthread.c

context.o: context.c context.h uthread.h preempt.h
	gcc -Wall -Werror -c -o context.o context.c

preempt.o: preempt.c preempt.h uthread.h
	gcc -Wall -Werror -c -o preempt.o preempt.c

clean: rm *.o *.a