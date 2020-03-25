#define _GNU_SOURCE
#include<sched.h>

#include<stdio.h> /* For clone */
#include<signal.h> /* For SIGCHLD */
#include<stdlib.h>
#include<sys/types.h> /* For pid_t */
#include<sys/wait.h> /* For wait */
#include<unistd.h> /* For getpid */
#include<stdbool.h> /* For bool*/
#include<stdatomic.h> /* For atomic_test_and_set*/

#define THREAD_STACK 1024*64
#define MAX_THREADS 10

static int thread_count = 0;

typedef struct thread_spinlock_t {
	bool flag;
}thread_spinlock_t;

typedef struct thread_t {
	int tid;
	int pid;
}thread_t;

typedef struct kernel_thread_t{
	pid_t pid;
	int tid;
	void *stack;
	struct kernel_thread_t *next, *prev;
}kernel_thread_t;

typedef struct function_container{
	void *(*function)();
	void *arg;	
}function_container;

kernel_thread_t *head;

void init(){
	head = NULL;
}

kernel_thread_t* newNode(){
	kernel_thread_t *p = (kernel_thread_t*)malloc(sizeof(kernel_thread_t));
	if(p == NULL){
		printf("Memory allocation error\n");
		exit(0);
	}
	p -> stack = malloc(THREAD_STACK);
	if(p -> stack == NULL){
		printf("Memory allocation error\n");
		exit(0);
	}
	p -> next = p -> prev = p;
	return p;
}

void add(kernel_thread_t *p){
	if(head == NULL){
		head = p;
		// printf("*\n**\n");
		return;
	}
	
	head -> prev -> next = p;
	p -> prev = head -> prev;
	head -> prev = p;
	p -> next = head;
}

kernel_thread_t* removeNode(int tid)
{
	if(head == NULL)
		return NULL;
	kernel_thread_t* p = head;
	if(p -> next == p){
		if(p -> tid == tid)
		{	
			head = NULL;
			return p;
		}
		return NULL;
	}
	do{
		if(p -> tid == tid){
			p -> prev -> next = p -> next;
			p -> next -> prev = p -> prev; 
			return p;
		}
		p = p -> next;
	}while(p != head);
	return NULL;
}

kernel_thread_t* removeNodeUsingPid(int pid)
{
	if(head == NULL)
		return NULL;
	kernel_thread_t* p = head;
	if(p -> next == p){
		if(p -> pid == pid){
			head = NULL;
			return p;
		}
		return NULL;
	}
	do{
		if(p -> pid == pid){
			p -> prev -> next = p -> next;
			p -> next -> prev = p -> prev; 
			return p;
		}
		p = p -> next;
	}while(p != head);
	return NULL;
}

void print(){
	kernel_thread_t *p = head;
	do{
		printf("* %d %d\n", p -> pid, p -> tid);
		p = p -> next;
	}while(p != head);
}

int function_execution(void *execution_args) {
	function_container *temp = (function_container*)execution_args;
	temp -> function(temp -> arg);
	return 0;
}

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {
	// if(thread_count == MAX_THREADS)
	// 	return -1;

	kernel_thread_t *p = newNode();
	add(p);

	// init_thread(&thread_array[thread_count]);

	/*Allocating stack*/
	// thread_array[thread_count].stack = malloc(THREAD_STACK);
	// if(thread_array[thread_count].stack == NULL)
	// 	return -1;

	function_container *execution_args;
	execution_args = (function_container*)malloc(sizeof(function_container));  //if error
	execution_args->function = start_routine;
	execution_args->arg = arg;

	/*Calling clone to create thread*/
	p -> pid = clone(&function_execution, (char*) p -> stack + THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, (void*)execution_args);
	thread -> pid = p -> pid;
	if(p -> pid == -1) {
		free(p -> stack);
		free(execution_args);
		return -1;
	}
	p -> tid = thread_count;
	thread -> tid = thread_count;
	thread_count++;
	return 0;	
}

int thread_join(thread_t thread) {
	int curr = thread.tid;
	int i;
	int status;
	waitpid(thread.pid, &status, 0);
	if(&status == NULL)
		return 0;
	kernel_thread_t *p = removeNode(thread.tid);

	free(p -> stack);
	p -> stack = NULL;
	free(p);
	// int flag = 1;
	// /*Restoring thread array*/
	// for(i = 0; i < thread_count; i++) {
	// 	if(thread_array[i].stack != NULL)
	// 		flag = 0;	
	// }
	// if(flag) {
	// 	thread_count = 0;	
	// }
	return 0;
}

int thread_kill(thread_t thread, int signo){
	return kill(thread.pid, signo);
}

void thread_exit(void *retval){
	int nowpid = getpid();
	kernel_thread_t *p = removeNodeUsingPid(nowpid);
	printf("\n&*\n");
	if(p == NULL)
		exit(0);

	// if(kill(nowpid, 9) == -1)
	// {
	// 	retval = (void*)-1;
	// 	return;
	// }
	
	// free(p -> stack);
	// p -> stack = NULL;
	// retval = (void*)0;
	// return;


	// for(int i = 0; i < MAX_THREADS; i++){
	// 	if(thread_array[i].pid == nowpid){
	// 		if(kill(nowpid, 9) == -1)
	// 		{

	// 			retval = (void*)-1;
	// 			return;
	// 		}
	// 		free(thread_array[i].stack);
	// 		thread_array[i].stack = NULL;
	// 		retval = (void*)0;
	// 		return;
	// 	}
	// }
	// Terminate the main thread
}
/*
void thread_exit(void *retval){
	int nowpid = getpid();
	kernel_thread_t *p = removeNodeUsingPid(nowpid);
	printf("**\n*")
	if(p == NULL)
		return;
	if(kill(nowpid, 9) == -1)
	{
		retval = (void*)-1;
		return;
	}
	free(p -> stack);
	p -> stack = NULL;
	retval = (void*)0;
}
*/
int thread_spin_init(thread_spinlock_t *lock) {
	lock->flag = false;
	return 0;
}
int thread_spin_lock(thread_spinlock_t *lock) {
	while(atomic_flag_test_and_set(&(lock->flag)) == true);
	return 0;

}
int thread_spin_unlock(thread_spinlock_t *lock) {
	lock->flag = false;
	return 0;
}
