targets := libuthread.a
objs    := queue.o uthread.o context.o preempt.o

CC      := gcc
CFLAGS  := -Wall -Werror
AR      := ar
ARFLAGS := rcs

ifneq ($(V),1)
B = @
endif

all: $(targets)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

libuthread.a: $(objs)
	@echo "AR   $@"
	$(B)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@echo "CC   $@"
	$(B)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean: 
	@echo "CLEAN"
	$(B)rm -f $(objs) $(targets) $(deps)