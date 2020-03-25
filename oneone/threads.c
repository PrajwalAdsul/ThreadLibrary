#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h> 
#include<signal.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<asm-generic/errno.h>

#define THREAD_STACK 1024*64
#define MAX_THREADS 10

static int thread_count = 0;

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
		p = p -> next;
	}while(p != head);
}

int function_execution(void *execution_args) {
	function_container *temp = (function_container*)execution_args;
	temp -> function(temp -> arg);
	return 0;
}

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {
	kernel_thread_t *p = newNode();
	add(p);
	function_container *execution_args;
	execution_args = (function_container*)malloc(sizeof(function_container));  //if error
	execution_args->function = start_routine;
	execution_args->arg = arg;
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
	if(status == NULL)
		return 0;
	kernel_thread_t *p = removeNode(thread.tid);
	free(p);

	/*
	if(deadlock)
		return EDEADLK;
	else if(notJoinableThread)
		return EINVAl;
	else if(anotherThreadAlreadyWaitingToJoinWithThisThread)
		return EINVAL;
	else if(noThreadWithIDThread)
		return ESRCH;
	*/


	return 0;
}

int thread_kill(thread_t thread, int signo){
	return kill(thread.pid, signo);
}

static int nowpid;
void thread_exit(void *retval){
	nowpid = getpid();
	kernel_thread_t *p = removeNodeUsingPid(nowpid);
	if(p == NULL)
		return;
	kill(nowpid, 9);
	p -> stack = NULL;
	retval = (void*)0;
	return;
}
