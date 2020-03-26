#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h> 
#include<signal.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<asm-generic/errno.h>
#include<stdbool.h> /* For bool*/
#include<stdatomic.h> /* For atomic_test_and_set*/
#include<setjmp.h>
#define THREAD_STACK 1024*64
#define MAX_THREADS 10

static int thread_count = 0;

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
	// int ret;
	bool retflag;  // true if value returned using return statement, false if return using thread_exit
}kernel_thread_t;

kernel_thread_t *head;

typedef struct thread_spinlock_t {
	bool flag;
}thread_spinlock_t;

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


kernel_thread_t* getNodeUsingPid(int pid)
{
	if(head == NULL)
		return NULL;
	kernel_thread_t* p = head;
	if(p -> next == p){
		if(p -> pid == pid){
			return p;
		}
		return NULL;
	}
	do{
		if(p -> pid == pid){
			return p;
		}
		p = p -> next;
	}while(p != head);
	return NULL;
}

void print(){
	kernel_thread_t *p = head;
	int i = 0;
	do{
		p = p -> next;
		// printf("print    %d    %d\n", p -> pid, p -> tid);
		i += 1;
		if(i == 2)
			break;
	}while(p != head);
}

int function_execution(void *p) {
	kernel_thread_t *q = (kernel_thread_t*)p;
	function_container *temp = (function_container*)(q -> fcont);
	// printf("start");
	// temp -> function(temp -> arg);
	q -> retval = temp -> function(temp -> arg);
	
	// printf("\n%d &&\n", (int*)(((kernel_thread_t*)p) -> retval));
	
	// printf("\n*%d\n", (int*)(((kernel_thread_t*)p) -> retval));
	// printf("end");
	return 0;
}

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {
	kernel_thread_t *p = newNode();
	add(p);
	// function_container *execution_args;
	// execution_args = (function_container*)malloc(sizeof(function_container));  //if error
	// execution_args->function = start_routine;
	// execution_args->arg = arg;

	p -> fcont = (function_container*)malloc(sizeof(function_container));
	p -> fcont -> function = start_routine;
	p -> fcont -> arg = arg;
	p -> retval = NULL;
	p -> pid = clone(&function_execution, (char*) p -> stack + THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, (void*)(p));
	p -> tid = thread_count;
	p -> retflag = true;
	// printf("**** %d %d\n", p -> pid, p -> tid);
	thread -> pid = p -> pid;
	if(p -> pid == -1) {
		free(p -> stack);
		free(p -> fcont);
		return -1;
	}
	thread -> tid = thread_count;
	thread_count++;
	return 0;	
}

int thread_join(thread_t thread, void **np) {
	int curr = thread.tid;
	int i;
	int status;
	waitpid(thread.pid, &status, 0);
	if(&status == NULL)
		return 0;
	kernel_thread_t *p = removeNode(thread.tid);
	// printf("^&^&\n");
	// if(*n != NULL)
	// printf("^^^6\n");
	// printf("p -> retval %x\n", (int*)p -> retval);
	// if(p )

	if(p != NULL){
		if(p -> retflag == true)
		{
			*np = &(p -> retval);
		}
		else{
			// printf("qw\n");
			// printf("qw   p -> retval %d\n", *(int*)(p -> retval));
			*np = (p -> retval);
		}
	}
	// printf("^^^6\n");
	// printf("@@%d\n", *((int*)*n));
	// printf("&%d\n", *(int**)n);
	// printf("**** %d %d\n", p -> pid, p -> tid);
	// free(p);

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

	// printf("Hn\n");
	nowpid = getpid();
	// printf("pid of thread %d\n", getpid());
	// printf("Hn\n");
	kernel_thread_t *p = getNodeUsingPid(nowpid);
	// printf("Hn\n");
	if(p == NULL)
		return;	
	// longjmp(buf, 1);
	// printf("Hn\n");
	p -> retflag = false;
	p -> retval = retval;
	// printf("Hn\n");
	// p -> ret = *(int*)retval;
	// printf("%d ret\n", sizeof(*retval));

	kill(nowpid, 9);
	// printf("Hn\n");
	p -> stack = NULL;

	// printf("&&%d\n", *(int*)(p -> retval));
	// printf("Hn\n");
	// printf("Hn\n");
}

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