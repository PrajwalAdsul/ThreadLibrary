#include "threads.h"
// keep track of how many user threads were created
static int thread_count = 0;

// 2 dimensional array to keep track of who which thread is waiting for which other threads
bool joinwait[MAX_THREADS][MAX_THREADS];

// linked list head to maintain a job queue
kernel_thread_t *head;

// a global variable to keep track of current process identifier
static int nowpid;

// initialise head to NULL and joinwait 2 dimensional array
void init(){
	head = NULL;
	int i, j;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			joinwait[i][j] = false;
		}
	}
}
// create a new TCB
kernel_thread_t* newNode(){
	kernel_thread_t *p = (kernel_thread_t*)malloc(sizeof(kernel_thread_t));
	// memory not available then exit
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
// add a job to job queue maintianing a doubly linked list structure
void add(kernel_thread_t *p){
	if(head == NULL){
		head = p;
		return;
	}	
	head -> prev -> next = p;
	p -> prev = head -> prev;
	head -> prev = p;
	p -> next = head;
}
// remove a node from queue identied by tid
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
	if(p -> tid == tid){
		head = p -> next;
		p -> prev -> next = p -> next;
		p -> next -> prev = p -> prev;
		return p;
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
// remove a node using associated pid
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
// just get the node, dont remove it
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
// get associated tid with a given pid
int gettidfrompid(int pid){
	if(head == NULL)
		return -1;
	kernel_thread_t* p = head;
	if(p -> next == p){
		if(p -> pid == pid){
			return p -> tid;
		}
		return -1;
	}
	do{
		if(p -> pid == pid){
			return p -> tid;
		}
		p = p -> next;
		
	}while(p != head);

	return -1;	
}
// used to print queue
void print(){
	kernel_thread_t *p = head;
	int i = 0;
	do{
		p = p -> next;
	}while(p != head);
}
// execute the start funciton
int function_execution(void *p) {
	kernel_thread_t *q = (kernel_thread_t*)p;
	function_container *temp = (function_container*)(q -> fcont);
	q -> retval = temp -> function(temp -> arg);
	return 0;
}
// for checking if another thread is already waiting to join with given thread
// used for error checking in thread join
bool anotherThreadAlreadyWaitingToJoinWithThisThread(int waitertid){
	for(int i = 0; i < 10; i++){
		if(joinwait[i][waitertid]){
			return true;
		}
	}
	return false;
}
// check if given thread identifier is valid
bool validthreadid(int tid){
	if(head == NULL)
		return -1;
	kernel_thread_t* p = head;
	if(p -> next == p){
		if(p -> tid == tid){
			return true;
		}
		return false;
	}
	do{
		if(p -> tid == tid){
			return true;
		}
		p = p -> next;
		
	}while(p != head);

	return false;	
}
// used to create a new user level thread and give it to clone system call for execution
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {
	kernel_thread_t *p = newNode();
	add(p);
	p -> fcont = (function_container*)malloc(sizeof(function_container));
	p -> fcont -> function = start_routine;
	p -> fcont -> arg = arg;
	p -> retval = NULL;
	p -> pid = clone(&function_execution, (char*) p -> stack + THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, (void*)(p));
	p -> tid = thread_count;
	p -> retflag = true;
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
// wait till the associated process with given thread gets completed
int thread_join(thread_t thread, void **np) {
	/*
	Error checking in join
		if(deadlock)
			return EDEADLK;
		else if(notJoinableThread)
			return EINVAl;
		else if(anotherThreadAlreadyWaitingToJoinWithThisThread)
			return EINVAL;
		else if(noThreadWithIDThread)
			return ESRCH;
	*/

	pid_t waitertid = -1;
	int nowpid = getpid();
	waitertid = gettidfrompid(nowpid);

	// check if thread with given id exists or not
	if(!validthreadid(thread.tid)){
		return ESRCH;
	}
	
	// Another thread waiting for this thread to join
	if(anotherThreadAlreadyWaitingToJoinWithThisThread(waitertid)){
		return EINVAL;
	}

	// Deadlock checking
	if(waitertid != -1){
		// printf("Here\n");
		if(joinwait[thread.tid][waitertid] == true){
			return EDEADLK;
		}
		joinwait[waitertid][thread.tid] = true;
	}
	int curr = thread.tid;
	int i;
	int status;
	// wait till process gets completed
	waitpid(thread.pid, &status, 0);
	if(&status == NULL)
		return 0;
	// remove the job from job queue
	kernel_thread_t *p = removeNode(thread.tid);
	/*Setting the return value once block is obtained from completed queue*/
	if(np != NULL)
	{
		if(p != NULL){
			// if return value was sent using normal return
			if(p -> retflag == true)
			{
				*np = &(p -> retval);
			}
			// if return value was send using thread_exit
			else{
				*np = (p -> retval);
			}
		}
	}
	joinwait[waitertid][thread.tid] = false;
	// free its contents
	free(p);
	return 0;
}
// pass a signal to given thread
int thread_kill(thread_t thread, int signo){
	return kill(thread.pid, signo);
}
// terminate the given thread and store the return value in TCB
void thread_exit(void *retval){
	nowpid = getpid();
	kernel_thread_t *p = getNodeUsingPid(nowpid);
	if(p == NULL)
		return;	
	// retflag is false when value is returned using thread_exit
	p -> retflag = false;
	p -> retval = retval;
	// kill the associated process
	kill(nowpid, 9);
	p -> stack = NULL;
}

// initialise a spinlock
int thread_spin_init(thread_spinlock_t *lock) {
	lock->flag = false;
	return 0;
}
// wait until lock is available and then take the lock
int thread_spin_lock(thread_spinlock_t *lock) {
	while(atomic_flag_test_and_set(&(lock->flag)) == true);
	return 0;
}
// leave the lock
int thread_spin_unlock(thread_spinlock_t *lock) {
	lock->flag = false;
	return 0;
}
