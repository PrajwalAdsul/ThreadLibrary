#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h> /* For clone */
#include<signal.h> /* For SIGCHLD */
#include<stdlib.h>
#include<sys/types.h> /* For pid_t */
#include<sys/wait.h> /* For wait */
#include<unistd.h> /* For getpid */

#define THREAD_STACK 1024*64
#define MAX_THREADS 10

typedef struct thread_t {
	int tid;
	int pid;
}thread_t;

typedef struct kernel_thread{
	pid_t pid;
	void *stack;
}kernel_thread;

typedef struct function_container{
	void *(*function)();
	void *arg;	
}function_container;

void init_thread(kernel_thread *t) {
	t -> pid = 0;
	t -> stack = NULL;	
}

static kernel_thread thread_array[MAX_THREADS];
static int thread_count = 0;

int function_execution(void *execution_args) {
	function_container *temp = (function_container*)execution_args;
	temp->function(temp->arg);
	return 0;
}

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {
	if(thread_count == MAX_THREADS)
		return -1;

	init_thread(&thread_array[thread_count]);

	/*Allocating stack*/
	thread_array[thread_count].stack = malloc(THREAD_STACK);
	if(thread_array[thread_count].stack == NULL)
		return -1;

	function_container *execution_args;
	execution_args = (function_container*)malloc(sizeof(function_container));
	execution_args->function = start_routine;
	execution_args->arg = arg;

	/*Calling clone to create thread*/
	thread_array[thread_count].pid = clone(&function_execution, (char*) thread_array[thread_count].stack + THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, (void*)execution_args);
	thread -> pid = thread_array[thread_count].pid;
	if(thread_array[thread_count].pid == -1) {
		free(thread_array[thread_count].stack);
		free(execution_args);
		return -1;
	}

	thread->tid = thread_count;
	thread_count++;
	 
	return 0;	
	
}

int thread_join(thread_t thread) {
	int curr = thread.tid;
	int i;
	int status;
	waitpid(thread_array[curr].pid, &status, 0);
	if(status == NULL)
		return 0;
	free(thread_array[curr].stack);
	thread_array[curr].stack = NULL;
	int flag = 1;
	/*Restoring thread array*/
	for(i = 0; i < thread_count; i++) {
		if(thread_array[i].stack != NULL)
			flag = 0;	
	}
	if(flag) {
		thread_count = 0;	
	}
	return 0;
}

int thread_kill(thread_t thread, int signo){
	return kill(thread.pid, signo);
}

void thread_exit(void *retval){
	int nowpid = getpid();
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread_array[i].pid == nowpid){
			if(kill(nowpid, 9) == -1)
			{

				retval = (void*)-1;
				return;
			}
			free(thread_array[i].stack);
			thread_array[i].stack = NULL;
			retval = (void*)0;
			return;
		}
	}
	// Terminate the main thread
	exit(0);
}