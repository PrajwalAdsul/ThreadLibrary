#ifndef THREADS_H
#define THREADS_H

#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h> 
#include<signal.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<unistd.h>
#include<asm-generic/errno.h>
#include<stdbool.h> /* For bool */
#include<stdatomic.h> /* For atomic_test_and_set */
#include<errno.h>
#include<time.h>
#include<sys/syscall.h>
#include<setjmp.h>
#include<sys/resource.h>
#include<assert.h>

#define KERNEL_THREAD_STACK (1024 * 1024)
#define DEFAULT_KERNEL_THREAD_COUNT 3
#define MAX_KERNEL_THREAD_COUNT 10
#define READY 1
#define TAKEN 2
#define COMPLETE 3 


// structure for pid container
typedef struct pid_container {
	int pid;
	jmp_buf pid_stage;
}pid_container;

// user level thread structure
typedef struct thread_t {
	int tid;
}thread_t;

// TCB structure for many to many
typedef struct mtm_thread_node {
	int tid;
	int pid;
	jmp_buf context;
	void *(*start_routine) (void *);
	void *arg;
	int status;
	void *stack;
	void *retval;
	bool retflag;
	struct mtm_thread_node *next;	
}mtm_thread_node;

// structure for spinlock
typedef struct thread_spinlock_t {
   bool flag;
}thread_spinlock_t;
// queue structure
typedef struct queue {
	mtm_thread_node *head;
	mtm_thread_node *tail;
}queue;


// scheduler function to check if there is job in queue to be executed
// all kernel threads are cloned by keeping this function as its arguments
int scheduler(void*);

// get container from pid
pid_container* get_pid_container(int pid);

// initialise spinlock
int thread_spin_init(thread_spinlock_t *lock);
// wait until lock is available and then take the lock
int thread_spin_lock(thread_spinlock_t *lock);
// leave the lock
int thread_spin_unlock(thread_spinlock_t *lock);
// initialise queue
int init_queue(queue *q);
// enqueue a job in queue
int enqueue(queue *q, mtm_thread_node *temp);
// remove front job from queue
mtm_thread_node* dequeue(queue *q);
// remove job using its associated tid
mtm_thread_node* queueRemoveUsingThreadID(queue *q, int id);
// remove job using its associated process identifier
mtm_thread_node* queueRemoveUsingProcessID(queue *q, int id);
// check if queue is empty
int isempty(queue q);
// print the queue
void printq(queue q);

// block sigalrm, required when you want to ignore the alarm
// code starting form block_sigalrm till end of unblocl_sigalrm will execute without alarm hindrance in between
void block_sigalrm();
// unblock sigalrm
void unblock_sigalrm();
// handler for SIGALRM
// for scheduling purpose
void handle_sigalrm();
// initialise timer
static bool init_timer();
// execute the start function and other functionalities
void execute(int signum);
// initialise library
void thread_init(int numberOfKernelThreads);
// used to create a new thread
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg);
// used to check if there is any job in the queue
int scheduler(void* args);
// wait till the job is completed and then return the value returned using return statement or thread_exit
int thread_join(thread_t thread, void **np);

#endif