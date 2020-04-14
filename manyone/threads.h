#ifndef THREADS_H
#define THREADS_H

// include all the essential header files
#include<assert.h>
#include<setjmp.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/time.h>
#include<sys/resource.h>
#include <stdatomic.h> /* For atomic_test_and_set*/
#include<errno.h>

// size for eact thread stack
#define THREAD_STACK (1024*64)

// defining maximum number of threads
#define MAX_THREADS 50

// user level thread structure
typedef struct thread_t {
	int tid; // id for user thread
}thread_t;
// thread control block
typedef struct user_thread_node {
	int tid; // id 
	jmp_buf context; // use to save contexts
	void *(*start_routine) (void *);  // pointer to start function
	void *arg; // pointer arguments of start function
	int active; // check if active
	void *stack; // pointer to stack of that thread
	void *retval;  // pointer to return value sent by start_function
	bool retflag; 		// used to check if value was returned using normal return or using thread_exit
	// if using normal return then this value set to true, if with thread_exit then this value set to false
	struct user_thread_node *next;	// pointer to next job
	int signo; // storing signo if send from thread_kill
}user_thread_node;

// structure for job queue
typedef struct queue {
	user_thread_node *head;
	user_thread_node *tail;
}queue;

// structure for spinlock
typedef struct thread_spinlock_t {
	bool flag;	// flag to decide whether lock is available or not
}thread_spinlock_t;

/**** functions for queue ****/
//init double headed queue
int init_queue(queue *q);
// add an element to a queue at end
int enqueue(queue *q, user_thread_node *temp);
// dequeue front element of queue
user_thread_node* dequeue(queue *q);
// remove a structure from queue using given thread id
user_thread_node* queueRemoveUsingID(queue *q, int id);
// check if thread is empty
int isempty(queue q);

/**** Spinlock ****/ 
// initialise a spinlock
int thread_spin_init(thread_spinlock_t *lock);
// wait until lock is available and then take the lock
int thread_spin_lock(thread_spinlock_t *lock);
// leave the lock
int thread_spin_unlock(thread_spinlock_t *lock);
// block sigalrm, required when you want to ignore the alarm
// code starting form block_sigalrm till end of unblocl_sigalrm will execute without alarm hindrance in between

/**** functions for internal system for scheduling ****/
void block_sigalrm();
// unblock sigalrm
void unblock_sigalrm();
// signal a particular thread
void init_main_context();
// signal handler for SIGALRM signal
// scheduler will be called at every alarm is SIGARLM is not blocked
void scheduler();
// used to set timer
static bool init_timer();
// call the start function and handle thread exit
// once the jobs is done then put it in completed queue
void execute(int signum);
// jump to point before the start function is called by above execute function

int invalidSigNo(int signo);
// check whether a thread with given tid is completed
// if job is completed then it will be found in completed queue
bool isThreadCompleted(int tid);
// signal a particular thread

/**** actual thread functions ****/
// used to create a new thread
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg);
// wait till the current thread exceutes and free all its memory
// return the value given as argument in thread_exit or simply returned from threads start_function
int thread_join(thread_t thread, void **np);
// check if signal no send from thread_kill is correct
int thread_kill(thread_t thread, int signo);
// thread_exit will skip the stop the current thread execution
int thread_exit(void *retval);

#endif