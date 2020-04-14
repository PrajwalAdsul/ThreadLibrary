#define _GNU_SOURCE
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
#define THREAD_STACK 1024*64
#define MAX_THREADS 10

typedef struct thread_t {
	int tid;
	int pid;
}thread_t;

typedef struct function_container{
	void *(*function)();
	void *arg;
}function_container;

typedef struct kernel_thread_t{
	pid_t pid;
	int tid;
	void *stack;
	struct kernel_thread_t *next, *prev;
	function_container *fcont;
	void *retval;
	bool retflag;  // true if value returned using return statement, false if return using thread_exit
}kernel_thread_t;

typedef struct thread_spinlock_t {
	bool flag;
}thread_spinlock_t;


void init();

kernel_thread_t* newNode();

void add(kernel_thread_t *p);

kernel_thread_t* removeNode(int tid);

kernel_thread_t* removeNodeUsingPid(int pid);

kernel_thread_t* getNodeUsingPid(int pid);

int gettidfrompid(int pid);

void print();

int function_execution(void *p);

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg);

bool anotherThreadAlreadyWaitingToJoinWithThisThread(int waitertid);

bool validthreadid(int tid);

int thread_join(thread_t thread, void **np);
int thread_kill(thread_t thread, int signo);

void thread_exit(void *retval);

int thread_spin_init(thread_spinlock_t *lock);

int thread_spin_lock(thread_spinlock_t *lock);

int thread_spin_unlock(thread_spinlock_t *lock);
