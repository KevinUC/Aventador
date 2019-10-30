# Queue

## 1.1 Queue Implementation

Our queue is essentially a doubly linked list. The list contains one or more
nodes. Beside storing data, each node also contains pointers to its previous 
and next node. The queue struct maintains pointers to the head, tail nodes and 
the length of the queue, so that it can add/remove nodes from two ends. To 
create a queue, we first dynamically allocate memory for the queue struct as 
well as its head and tail nodes. We then initialize the queue by setting its 
length variable to zero and making its head and tail nodes pointing to each 
other. When enqueuing a data, we first create a new node to store the data, 
and then insert the node in between the head node and the current first node,
before finally incrementing the queue length variable by one. Similarly, 
dequeuing simply requires the queue to connect the second to last node with 
the tail, thereby removing the current last node from queue. The queue will
then decrement the length variable by 1 and makes the data argument store the 
address of the removed data. To delete a data, the queue will first iterate 
all the nodes from the last node and removes the first node that contains the 
exact same data by resetting the prev and next pointers of its neighbors. If 
the data is not found, the length variable will not be decremented. The 
**queue_iterate()** interface is implemented by looping over all the nodes 
from the last one and apply the provided function along the way. If the 
function returns 1 and the data argument is not NULL, it will make the data 
argument store the address to the data of the current node before quitting the 
function. Lastly, the **queue_length()** interface simply returns the length 
field of the queue.

## 1.2 Testing the queue

We created 13 unit tests in test_queue.c. They can be categorized in two 
groups: error handling testers and functionality testers. The first group only
checks if the interface handles errors properly. For example, 
**testEnqueueErrorHandle()** checks if the enqueue interface returns -1 when 
the provided queue or the data is NULL. The second group focuses on testing 
the functionalities. For example, **testIterate()** first populates the queue 
with an int array and then apply different callback functions on the queue by 
calling **queue_iterate()**. At the end, the function will use **assert()** to 
check if the values of array elements and other variables agree with the 
expected outputs.

# uthread library

## 2.1 Data structures

All the information of a thread is stored in its TCB. The TCB struct contains 
the thread’s tid, context, pointer to top of the stack and the its return 
value. The TCBController manages the states of all the threads by maintaining 
three TCB queues: \_readyQueue, \_blockedQueue and \_zombieQueue. As the names 
suggest, each queue stores TCB pointers of the threads that are currently in 
the corresponding state. In addition, the TCBController contains the tid array 
\_JoinedBy, in which each entry stores the tid of the thread that is blocked 
by the corresponding thread. The \_isTidValid boolean array marks if the 
thread with the given tid still exists (in ready, blocked or zombie state). 
The \_runningTd filed points to the TCB of the currently running thread. 
Finally, \_count indicates the total number of threads being registered so far 
(including dead ones). Since the information stored in tcbController will be 
used by almost all the library interfaces, we declared it as a global variable.

## 2.2.1 Non-preemptive mode (Phase 2-3)

When called the first time, **uthread_create()** will call **uthread_init()** 
to initialize all the fields in tcbController, and then create a TCB for the 
main thread before adding it to the ready queue. It will then create a TCB for
the newly created thread and insert it to the ready queue. To yield the 
running thread, the library will first enqueue its TCB to the ready queue, and 
then invoke **switchToFirstReadyTd()**, which dequeues from the ready queue, 
updates the controller’s runningTd field with the dequeued TCB, and finally 
invokes switches context by invoking **uthread_ctx_switch()**. To join with 
another thread T1, the library needs to check if T1 is valid before proceeds.
If the TCB of thread T1 is already in the zombie queue, we only need to 
release its resource and mark T1 as invalid tid. Otherwise, we need to put the 
current Thread T0 into the blocked queue and immediately calls 
**switchToFirstReadyTd()** to run the next available thread. When T0 is waken
up and scheduled to run again, it can finally collect the resource used by T1
and continue with the remaining executions. To implement **uthread_exit()**, 
we first check if the currently running thread T0 is joined by some other 
thread. We did that by checking if JoinedBy[T0] is smaller than USHRT_MAX, the 
default value set for each entry during the initialization phase. Then, if it 
is smaller, we need to remove TCB of the thread JoinedBy[T0] from the blocked 
queue and insert it back to the ready queue so that thread JoinedBy[T0] will 
be scheduled to run sometime in future. The last step of **uthread_exit()** is 
to invoke **switchToFirstReadyTd()**, so that the current thread will be 
removed from the ready queue and the next available thread will start running.

## 2.2.2 Testing Non-preemptive mode

Besides testing against the original uthread_hello.c and uthread_yield.c and 
matching the expected outputs, we also modified uthread_yield.c by letting 
thread 2 join thread 1 and letting thread 3 join thread 2. The obtained 
results match the expected ones.

## 2.3.1 Preemptive mode (Phase 4)

**preempt_start()**: To start the preemption mechanism, we specify how the 
SIGVTALRM signal should be handled by declaring **sigvtalnHandler()**, 
which simply calls **uthread_yield()** to schedule the next thread. We 
then initializes the sigaction struct and calls **sigaction()** to activate
the handler when the signal is received. The second step is to start sending 
the signal. We initialize a itimeral struct by setting its tv_usec field to 
(1000/HZ) \*1000 so that the timer will expire 100 times within a second. We 
then invokes **setitimer()** to trigger the virtual timer. **preempt_start()**
will be called exactly once during the first invocation of the libbrary 
function **uthread_create()**.

**preempt_disable()** : To temporarily disable the preemption, we first
initialize the signal set by making SIGVTALRM as its only member. we then 
invoke **sigpromask()** with the flag SIG_BLOCK, so that SIGVTALRM will be 
blocked for the current thread.

**preempt_enable()** : The implementation is almost identical to the 
**preempt_disable()**, except that this time we want to remove SIGVTALRM from 
the current signal set by changing the flag SIG_BLOCK to SIG_UNBLOCK.

**preempt_enable()** and **preempt_disable()** only need to called when the 
library function is reading/modifying the shared resource or performing 
context switch. In our case, the shared resource will be the global variable 
tcbController. Thus, operations on tcbController need to be protected by 
temporarily disabling the preemption. For example, we need to disable 
preemption before calling **isTidValid()** because this function relies on the
\_isTidValid field of the tcbController variable to function properly. If 
another thread interrupts during the checking process by modifying the array,
then the results will become non-deterministic.

## 2.3.2 Preemptive mode testing

In test_preempt.c, the main thread will create three new threads and each 
thread Tn will print “thread n” to the console. Since the thread does not call 
**uthread_yield()**, they rely on the preemption mechanism to yield control. 
The output consists of sequences of messages printed by each of the three 
threads until all threads have finished executing, which indicates the 
preemption mechanism functions normally and the underlying tcbController is
well protected from context switch.

## 3 External references

Besides referencing all the links provided in the assignment prompt and many 
piazza posts, we also looked up some external resources to complete this 
assignment:

linuxprogrammingblog.com/code-examples/blocking-signals-with-sigprocmask

https://pubs.opengroup.org/onlinepubs/7908799/xsh/sigemptyset.html

http://www.informit.com/articles/article.aspx?p=23618&seqNum=14
