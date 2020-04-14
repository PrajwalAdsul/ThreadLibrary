#ifndef THREADS_H
#define THREADS_H

#define _GNU_SOURCE

// include all the essential header files
#include <sched.h>
#include <stdio.h> 
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <asm-generic/errno.h>
#include <stdbool.h> /* For bool*/
#include <stdatomic.h> /* For atomic_test_and_set*/
#include <setjmp.h>
#include <errno.h>

// size for eact thread stack
#define THREAD_STACK 1024*64

// defining maximum number of threads
#define MAX_THREADS 10

// user level thread sturcture
typedef struct thread_t {
	int tid;  // thread identifier
	int pid;  // associated process identifier with respective tid
}thread_t;

// function container structure to store function pointer and args that are to be sent to it
typedef struct function_container{
	void *(*function)();
	void *arg;
}function_container;

// structure for kernel thread
typedef struct kernel_thread_t{
	pid_t pid; // process identifier
	int tid; // associated thread identifier
	void *stack;  // pointer to thread stack
	struct kernel_thread_t *next, *prev; // next and previous pointer to same type of structure
	function_container *fcont;	// function container for function execution
	void *retval;	// pointer to return value
	bool retflag;  // true if value returned using return statement, false if return using thread_exit
}kernel_thread_t;

// structure for spinlock
typedef struct thread_spinlock_t {
	bool flag;	// flag to decide whether lock is available or not
}thread_spinlock_t;

// initialise 2 dimensional array that is used to keep track of who which thread is waiting for which other threads
// also initialise head of linked list to NULL
void init();
// create a new TCB
kernel_thread_t* newNode();
// add a job to job queue maintianing a doubly linked list structure
void add(kernel_thread_t *p);
// remove a node from queue identied by tid
kernel_thread_t* removeNode(int tid);
// remove a node using associated pid
kernel_thread_t* removeNodeUsingPid(int pid);
// just get the node, dont remove it
kernel_thread_t* getNodeUsingPid(int pid);
// get associated tid with a given pid
int gettidfrompid(int pid);
// used to print queue
void print();
// execute the start funciton
int function_execution(void *p);
// for checking if another thread is already waiting to join with given thread
// used for error checking in thread join
bool anotherThreadAlreadyWaitingToJoinWithThisThread(int waitertid);
// check if given thread identifier is valid
bool validthreadid(int tid);
// used to create a new user level thread and give it to clone system call for execution
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg);
// wait till the associated process with given thread gets completed
int thread_join(thread_t thread, void **np);
// pass a signal to given thread
int thread_kill(thread_t thread, int signo);
// terminate the given thread and store the return value in TCB
void thread_exit(void *retval);
// initialise a spinlock
int thread_spin_init(thread_spinlock_t *lock);
// wait until lock is available and then take the lock
int thread_spin_lock(thread_spinlock_t *lock);
// leave the lock
int thread_spin_unlock(thread_spinlock_t *lock);
#endif