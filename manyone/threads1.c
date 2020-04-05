#include<assert.h>
#include<setjmp.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<errno.h>

#define THREAD_STACK (1024*64)
#define MAX_THREADS 10

static int thread_count = 0;

jmp_buf buf[MAX_THREADS];



typedef struct thread_t {
	int tid;
}thread_t;

typedef struct user_thread_node {
	int tid;
	jmp_buf context;
	void *(*start_routine) (void *);
	void *arg;
	int active;
	void *stack;
	void *retval;
	bool retflag;
	struct user_thread_node *next;	
	int signo;
	int exit;
}user_thread_node;

typedef struct queue {
	user_thread_node *head;
	user_thread_node *tail;
}queue;

int init_queue(queue *q) {
	q->head = NULL;
	q->tail = NULL;
	return 0;
}

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

int isempty(queue q) {
	return q.tail == NULL;
}

void printq(queue q) {
	user_thread_node *temp;
	if(q.tail == NULL) {
		return;
	}
	temp = q.head;
	while(temp->next != q.head) {
		temp = temp->next;	
	}
	return;
}

queue ready;
queue completed;
user_thread_node *current;
user_thread_node *setter;
int sync_flag = 1;

void block_sigalrm() {
	sigset_t sigalrm;
	sigemptyset(&sigalrm);
	sigaddset(&sigalrm, SIGALRM);
	sigprocmask(SIG_BLOCK, &sigalrm, NULL);
}


void unblock_sigalrm() {
	sigset_t sigalrm;
	sigemptyset(&sigalrm);
	sigaddset(&sigalrm, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigalrm, NULL);
}

void createStackForThreadandExecute(int signum) {
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
		}
		current->active = 0;		
		block_sigalrm();
		printq(ready);
		enqueue(&completed, current);
		current = dequeue(&ready);
		//unblock_sigalrm();
		longjmp(current->context, 1);
	}
	return;
}

int thread_exit(void *retval){
	block_sigalrm();
	longjmp(buf[current -> tid], 1);
	// enqueue(&completed, current);
	// current = dequeue(&ready);
	unblock_sigalrm();
}
bool invalidSigNo(signo){
	return false;
}

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

int thread_kill(thread_t thread, int signo)
{
	block_sigalrm();
	// If signal no is invalid then return false
	if(invalidSigNo(signo)){
		return EINVAL;
	}	
	if(current != NULL){
		if(current -> tid != thread.tid){
			raise(signo);
			// current = queueRemoveUsingID(&ready, thread.tid);
			// if(current == NULL){
			// 	return 0;
			// }
			// else{
			// 	current -> signo = signo;
			// 	enqueue(&ready, current);
			// }
		}
	}
	unblock_sigalrm();
	return 1;
}
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
	thread_count++;
	return;
}

void handle_sigalrm() {
	if(setjmp(current->context) && sync_flag == 2) {
		sync_flag = 1;	
		return;
	}
	else {
		int flag; 
		if(current->tid == 0)
			flag = 1;
		// For thread_kill 
		// if(current -> tid != 0 && current -> signo != -1 && current -> signo != 0){
		// 	raise(current -> signo);
		// 	current -> signo = -1;
		// }
		enqueue(&ready, current);
		current = dequeue(&ready);
	
		if(flag == 1 && current->tid == 0)
			return;
		
		sync_flag = 2;	
		longjmp(current->context, sync_flag);
	}
	return;
}

struct itimerval timer;
static bool init_timer() {
	/*Installing signal handler*/
	sigset_t all;
	//sigfillset(&all);
	sigemptyset(&all);
	sigaddset(&all, SIGALRM);

	const struct sigaction alarm = {
		.sa_handler = &handle_sigalrm,
		.sa_mask = all,
		.sa_flags = SA_RESTART
	};

	struct sigaction old;
	if(sigaction(SIGALRM, &alarm, &old) == -1);


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
	temp -> exit = 0;

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
	handler.sa_handler = &createStackForThreadandExecute;
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

int thread_join(thread_t thread, void **np) {
	int tid = thread.tid;
	user_thread_node *temp;

	/*Checking again and again if given id is in completed queue*/
	while(1) {
		block_sigalrm();
		temp = queueRemoveUsingID(&completed, tid);
		unblock_sigalrm();
		if(temp != NULL)
			break;		
	}

	/*Setting the return value once block is obtained from completed queue*/
	if(np != NULL)
		*np = temp->retval;

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
	return tid;
}

