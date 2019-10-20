all: libuthread.a

libuthread.a: queue.o
	ar rcs libuthread.a queue.o

queue.o: queue.c queue.h
	gcc -Wall -Werror -c -o queue.o queue.c