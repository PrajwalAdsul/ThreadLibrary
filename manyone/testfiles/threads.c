#include "threads.h"

// Keeping te count of user threads
static int thread_count = 0;

// Used to jump out of context thread is executing
jmp_buf buf[MAX_THREADS];
//init double headed queue
int init_queue(queue *q) {
	q->head = NULL;
	q->tail = NULL;
	return 0;
}
// add an element to a queue at end
int enqueue(queue *q, user_thread_node *temp) {
	if(q->tail == NULL) {
		q->head = temp;
		q->tail = temp;
		temp->next = temp;
	}
	else {
		q->tail->next = temp;
		temp->next = q->head;
		q->tail = temp;			
	}
	return 0;
}
// dequeue front element of queue
user_thread_node* dequeue(queue *q) {
	user_thread_node *temp;
	int flag = 0;
	if(q->tail == NULL)
		return NULL;
	else {
		if(q->head == q->tail)
			flag = 1;
		temp = q->head;
		q->head = temp->next;
		q->tail->next = q->head;
		temp->next = NULL;
		if(flag) {
			q->head = NULL;
			q->tail = NULL;
		}
		return temp;
	}
}
// remove a structure from queue using given thread id
user_thread_node* queueRemoveUsingID(queue *q, int id) {
	if(q->tail == NULL)
		return NULL;
	int flag = 0;
	user_thread_node *temp;
	user_thread_node *prev;
	temp = q->head;	
	prev = q->tail;
	if(q->head == q->tail) 
		flag = 1;
	while(temp->next != q->head) {
		if(temp->tid == id)
			break;
		temp = temp->next;
		prev = prev->next;	
	}
	if(temp->tid != id)
		return NULL;
	prev->next = temp->next;
	temp->next = NULL;
	if(temp == q->head)
		q->head = prev->next;
	if(temp == q->tail)
		q->tail = prev;
	if(flag) {
		q->head = NULL;
		q->tail = NULL;	
	}
	return temp;
}
// check if thread is empty
int isempty(queue q) {
	return q.tail == NULL;
}
// ready queue will hold all the jobs that are ready to be executed and all the ongoing jobs
queue ready;
// completed queue hold the jobs that have finished their execution
queue completed;
// structure to hold current context
user_thread_node *current;
//
user_thread_node *setter;
//
int sync_flag = 1;
// block sigalrm, required when you want to ignore the alarm
// code starting form block_sigalrm till end of unblocl_sigalrm will execute without alarm hindrance in between
void block_sigalrm() {
	sigset_t sigalrm;
	sigemptyset(&sigalrm);
	sigaddset(&sigalrm, SIGALRM);
	sigprocmask(SIG_BLOCK, &sigalrm, NULL);
}
// unblock sigalrm
void unblock_sigalrm() {
	sigset_t sigalrm;
	sigemptyset(&sigalrm);
	sigaddset(&sigalrm, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigalrm, NULL);
}
// call the start function and handle thread exit
// once the jobs is done then put it in completed queue
void execute(int signum) {
	assert(signum == SIGUSR1);
	if(setter == NULL) {
		exit(0);	
	}
	/*Saving the current context, and returning to terminate the signal handler scope*/
	if (setjmp(setter->context)) {
		/*Being called by scheduler for first time. Call the function*/
		unblock_sigalrm();
		// Used when exit is called
		if(setjmp(buf[current->tid])){
		}
		else{
			current->retval = current->start_routine(current->arg);
			current -> retflag = true;
		}
		current->active = 0;		
		block_sigalrm();
		enqueue(&completed, current);
		current = dequeue(&ready);
		//unblock_sigalrm();
		longjmp(current->context, 1);
	}
	return;
}

// check if signal no send from thread_kill is correct
int invalidSigNo(int signo){
	return signo < 1 || signo > 65;
}

// check whether a thread with given tid is completed
// if job is completed then it will be found in completed queue
bool isThreadCompleted(int tid){
	user_thread_node *p = completed.head;
	while(p -> next != completed.head){
		if(tid == p -> tid){
			return true;
		}
		p = p -> next;
	}
	return false;
}

// initializing the main context
void init_main_context() {
	/*Creating node for main*/	
	user_thread_node *main;
	main = (user_thread_node*)malloc(sizeof(user_thread_node));
	
	/*Setting its values*/
	main->tid = thread_count;
	main->start_routine = NULL;
	main->arg = NULL;
	main->active = 1;	
	main->next = NULL;
	main->stack = NULL;
	
	/*Backing up main context*/
	if (setjmp(main->context)){
		unblock_sigalrm();
		return;
	}
	current = main;
	// main thread will always be the first thread
	thread_count++;
	return;
}
// signal handler for SIGALRM signal
// scheduler will be called at every alarm is SIGARLM is not blocked
void scheduler() {
	if(setjmp(current->context) && sync_flag == 2) {
		sync_flag = 1;	
		return;
	}
	else {
		int flag; 
		// if main context
		if(current->tid == 0)
			flag = 1;
		// put the current context to back of the queue
		enqueue(&ready, current);
		// put front job form queue into current context for its execution
		current = dequeue(&ready);
	
		if(flag == 1 && current->tid == 0)
			return;
		
		sync_flag = 2;	
		longjmp(current->context, sync_flag);
	}
	return;
}
// used to set timer
struct itimerval timer;
static bool init_timer() {
	/*Installing signal handler*/
	sigset_t all;
	//sigfillset(&all);
	sigemptyset(&all);
	sigaddset(&all, SIGALRM);

	const struct sigaction alarm = {
		.sa_handler = &scheduler,
		.sa_mask = all,
		.sa_flags = SA_RESTART
	};

	struct sigaction old;
	if(sigaction(SIGALRM, &alarm, &old) == -1);

	// using 10 microseconds as interval
	struct itimerval timer = {
		{ 0, 10 },	//it_interval
		{ 0, 1 }	//it_value
	};
	/*Enabling timer*/
	if (setitimer(ITIMER_REAL, &timer, NULL) == - 1) {
		sigaction(SIGALRM, &old, NULL);
		return false;
	}
	return true;
}
// used to create a new thread
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {

	block_sigalrm();
	
	/*Initializing everything if first thread*/
	if(thread_count == 0) {
		/*Initializing the queues*/
		init_queue(&ready);
		init_queue(&completed);
		/*Initializing the main context*/
		init_main_context();
		/*Initializing the timer for scheduling*/
		init_timer();
	}

	/*Creating node and allocating it memory*/
	user_thread_node *temp;
	temp = (user_thread_node*)malloc(sizeof(user_thread_node));	

	/*Setting its values*/
	temp->tid = thread_count;
	temp->start_routine = start_routine;
	temp->arg = arg;
	temp->active = 1;	
	temp->next = NULL;
	temp -> signo = -1;
	temp -> retflag = false;

	struct sigaction handler;
	struct sigaction oldHandler;
	
	stack_t stack;
	stack_t oldStack;
	
	/*Creating new stack*/
	struct rlimit limit;

	getrlimit(RLIMIT_STACK, &limit);
	stack.ss_flags = 0;
	stack.ss_size = limit.rlim_cur;
	stack.ss_sp = malloc(limit.rlim_cur);

	/*Installing the new stack for the signal handler*/
	sigaltstack(&stack, &oldStack);

	/*Installing the signal handler*/
	handler.sa_handler = &execute;
	handler.sa_flags = SA_ONSTACK;
	sigemptyset(&handler.sa_mask);

	sigaction(SIGUSR1, &handler, &oldHandler);

	/*To set the jump buffer of thread*/
	setter = temp;
	/* Calling the handler on the new stack */
	raise(SIGUSR1);
	setter = NULL;
	
	/* Restore the original stack and handler */
	sigaltstack(&oldStack, 0);
	sigaction(SIGUSR1, &oldHandler, 0);
	
	temp->stack = stack.ss_sp;

	/*Adding the thread to the ready queue*/
	enqueue(&ready, temp);
	thread->tid = temp->tid;
	thread_count++;
	
	unblock_sigalrm();
	return 0;	
}
// wait till the current thread exceutes and free all its memory
// return the value given as argument in thread_exit or simply returned from threads start_function
int thread_join(thread_t thread, void **np) {
	int tid = thread.tid;
	user_thread_node *temp;
	
	// if thread id is not correct, then send no such process errno
	if(tid >= thread_count)
		return ESRCH;

	/*Checking again and again if given id is in completed queue*/
	while(1) {
		block_sigalrm();
		temp = queueRemoveUsingID(&completed, tid);
		if(temp != NULL)
			break;		
		unblock_sigalrm();
	}

	/*Setting the return value once block is obtained from completed queue*/
	if(np != NULL)
	{
		if(temp != NULL){
			// if return value was sent using normal return
			if(temp -> retflag == true)
			{
				*np = &(temp -> retval);
			}
			// if return value was send using thread_exit
			else{
				*np = (temp -> retval);
			}
		}
	}

	/*Freeing the memory allocated to block*/
	if(temp->stack != NULL)
		free(temp->stack);
	free(temp);
	
	/*Freeing current and stopping timer if all  the threads are done executing*/
	if(isempty(ready) && isempty(completed)) {
		struct itimerval timer = {
			{ 0, 0 },
			{ 0, 0 }
		};
		/*Disarming timer*/
		setitimer(ITIMER_REAL, &timer, NULL);
		free(current);
		thread_count = 0;	
	}
	unblock_sigalrm();
	return tid;
}

// signal a particular thread
int thread_kill(thread_t thread, int signo)
{
	block_sigalrm();
	// If signal no is invalid then return false
	if(invalidSigNo(signo)){
		return EINVAL;
	}	
	if(current != NULL){
		// check if current context is same as the context of intended thread
		if(current -> tid != thread.tid){
			// raise the signal
			raise(signo);
		}
	}
	unblock_sigalrm();
	return 1;
}

// jump to point before the start function is called by above execute function
// thread_exit will skip the stop the current thread execution
int thread_exit(void *retval){
	block_sigalrm();
	current -> retval = retval;
	longjmp(buf[current -> tid], 1);
	unblock_sigalrm();
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
