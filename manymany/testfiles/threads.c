#include "threads.h"

// initialise required values
int thread_count = 0;
int kernel_thread_count = 0;
bool first = true;

pid_container process_array[MAX_KERNEL_THREAD_COUNT];
thread_spinlock_t lock, g;

// initialise spinlock
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
// initialise queue
int init_queue(queue *q) {
	q->head = NULL;
	q->tail = NULL;
	return 0;
}
// enqueue a job in queue
int enqueue(queue *q, mtm_thread_node *temp) {
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
// remove front job from queue
mtm_thread_node* dequeue(queue *q) {
	mtm_thread_node *temp;
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
// remove job using its associated tid
mtm_thread_node* queueRemoveUsingThreadID(queue *q, int id) {
	if(q->tail == NULL)
		return NULL;
	int flag = 0;
	mtm_thread_node *temp;
	mtm_thread_node *prev;
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
// remove job using its associated process identifier
mtm_thread_node* queueRemoveUsingProcessID(queue *q, int id) {
	if(q->tail == NULL)
		return NULL;
	int flag = 0;
	mtm_thread_node *temp;
	mtm_thread_node *prev;
	temp = q->head;	
	prev = q->tail;
	if(q->head == q->tail) 
		flag = 1;
	while(temp->next != q->head) {
		if(temp->pid == id)
			break;
		temp = temp->next;
		prev = prev->next;	
	}
	if(temp->pid != id)
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
// check if queue is empty
int isempty(queue q) {
	if(q.tail == NULL)
		return 1;
	else
		return 0;
}
// print the queue
void printq(queue q) {
	mtm_thread_node *temp;
	if(q.tail == NULL) {
		printf("\nEMPTY!\n");
		return;
	}
	temp = q.head;
	printf("[");
	while(temp->next != q.head) {
		printf("%d ", temp->tid);
		temp = temp->next;	
	}
	printf("%d]\n", temp->tid);
	return;
}
// 3 queue to maintain states of jobs
queue ready;
queue taken;
queue completed;
// below functionalities for scheduling purpose
mtm_thread_node *current = NULL;
mtm_thread_node *setter;
int sync_flag = 1;
thread_spinlock_t lock;
int terminate_flag = 0;

// stores pid of main
int main_pid; 

// get container from pid
pid_container* get_pid_container(int pid) {
	for(int i = 0; i < kernel_thread_count; i++) {
		if(process_array[i].pid == pid)
			return &process_array[i];
	}
	return NULL;
}

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
// handler for SIGALRM
// for scheduling purpose
void handle_sigalrm() {
	thread_spin_lock(&lock);
	if(current == NULL) {
		current = queueRemoveUsingProcessID(&taken, getpid());
		thread_spin_unlock(&lock);
		return;
	}
	if(isempty(ready)) {
		thread_spin_unlock(&lock);
		return;	
	}
	if(current->pid != getpid()) {
		enqueue(&taken, current);
		current = queueRemoveUsingProcessID(&taken, getpid());
		if(current == NULL) {
			current = dequeue(&ready);
			current->pid = getpid();
			sync_flag = 2;
			longjmp(current->context, sync_flag);		
		}	
	}	
	/* Storing the current context */
	if(setjmp(current->context) && sync_flag == 2) {
		sync_flag = 1;
		thread_spin_unlock(&lock);	
		return;
	}
	else {
		current->status = READY;
		current->pid = 0;		
		enqueue(&ready, current);
		current = dequeue(&ready);
		current->pid = getpid();		
		sync_flag = 2;	
		longjmp(current->context, sync_flag);
	}
	return;
}
// initialise timer
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
		{ 0, 10000 },	//it_interval
		{ 0, 1 }	//it_value
	};
	/*Enabling timer*/
	if (setitimer(ITIMER_REAL, &timer, NULL) == - 1) {
		sigaction(SIGALRM, &old, NULL);
		return false;
	}
	return true;
}
// execute the start function and other functionalities
void execute(int signum) {
	assert(signum == SIGUSR1);
	if(setter == NULL) {
		printf("Error\n");
		exit(0);	
	}
	/*Saving the current context, and returning to terminate the signal handler scope*/
	if (setjmp(setter->context)) {
		void *temp_retval;
		/*Being called by scheduler for first time. Call the function*/
		thread_spin_unlock(&lock);
		unblock_sigalrm();
		while(current == NULL);
		temp_retval = current->start_routine(current->arg);
		block_sigalrm();
		thread_spin_lock(&lock);
		if(current == NULL) {
			current = queueRemoveUsingProcessID(&taken, getpid());
		}			
		else if(current->pid != getpid()) {
			enqueue(&taken, current);
			current = queueRemoveUsingProcessID(&taken, getpid());
		}
		current->retval = temp_retval;		 
		current->status = COMPLETE;
		enqueue(&completed, current);
		current = dequeue(&ready);
		if(current != NULL) {
			current->pid = getpid();
			longjmp(current->context, 1);
		}
		else {
			pid_container *p;
			p = get_pid_container(getpid());
			longjmp(p->pid_stage, 1);	
		}
	}
	return;
}
// initialise library
void thread_init(int numberOfKernelThreads){
	if(first) {
		void* stacks[numberOfKernelThreads];
		int i;
		
		/*Allocating memory for kernel thread stacks*/
		for(i = 0; i < numberOfKernelThreads; i++)
			stacks[i] = malloc(KERNEL_THREAD_STACK);

		/*Using clone to create kernel thread*/
		for(i = 0; i < numberOfKernelThreads; i++){
			// call scheduler on each cloned thread
			int pid = clone(scheduler, stacks[i] + KERNEL_THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, NULL);  
			if(pid == -1) {
				printf("Maximum number of kernel threads execeeded\n");
				exit(0);
			}
			else {
				if(kernel_thread_count > MAX_KERNEL_THREAD_COUNT) {
					printf("Maximum number of kernel threads execeeded\n");
					exit(0);
				}
				process_array[kernel_thread_count++].pid = pid;			
			}
		}
		first = false;
	}
	return;
}
// used to create a new thread
int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg) {

	/*Initializing everything if first thread*/
	if(thread_count == 0) {
		main_pid = getpid();
		
		// uncomment below line to see that user level processes are processing on fix number of kernel threads
			// printf("[%d] is process is of main\n", getpid());
			/*Initializing the queues*/
		init_queue(&ready);
		init_queue(&taken);
		init_queue(&completed);

		/*Initializing the lock*/
		thread_spin_init(&lock);

		/*Creating default number of kernel threads*/
		if(first) {
			void* stacks[DEFAULT_KERNEL_THREAD_COUNT];
			int i;

			for(i = 0; i < DEFAULT_KERNEL_THREAD_COUNT; i++) 
				stacks[i] = malloc(KERNEL_THREAD_STACK);
			// make default number of kernel threads if user hasnt passed number of required kernel threads
			for(i = 0; i < DEFAULT_KERNEL_THREAD_COUNT; i++) {
				int pid = clone(scheduler, stacks[i] + KERNEL_THREAD_STACK, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, NULL);  
				if(pid == -1) {
					printf("Maximum number of kernel threads execeeded\n");
					exit(0);
				}
				else {
					if(kernel_thread_count > MAX_KERNEL_THREAD_COUNT) {
						printf("Maximum number of kernel threads execeeded\n");
						exit(0);
					}
					process_array[kernel_thread_count++].pid = pid;			
				}
			}
			first = false;
		}

	}
	
	thread_spin_lock(&lock);
	/*Creating node and allocating it memory*/
	mtm_thread_node *temp;
	temp = (mtm_thread_node*)malloc(sizeof(mtm_thread_node));
	/*Setting its values*/
	temp->tid = thread_count;
	temp->pid = 0;
	temp->start_routine = start_routine;
	temp->arg = arg;
	temp->status = READY;	
	temp->next = NULL;

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
	/*Calling the handler on the new stack*/
	raise(SIGUSR1);

	setter = NULL;
	
	/*Restoring the original stack and handler*/
	sigaltstack(&oldStack, 0);
	sigaction(SIGUSR1, &oldHandler, 0);
	
	temp->stack = stack.ss_sp;

	/*Adding the thread to the ready queue*/
	enqueue(&ready, temp);
	thread->tid = temp->tid;
	thread_count++;
	
	thread_spin_unlock(&lock);
	//unblock_sigalrm();
	return 0;	
}
// used to check if there is any job in the queue
int scheduler(void* args) {
	/*Initializing the timer for scheduling*/
	block_sigalrm();
	init_timer();
	//unblock_sigalrm();	
	while(1) {
		block_sigalrm();
		thread_spin_lock(&lock);
		if(!isempty(ready)) {
			if(current != NULL) {
				current->status = TAKEN;
				enqueue(&taken, current);
			}
			current = dequeue(&ready);
			current->pid = getpid();
			pid_container *p;
			p = get_pid_container(getpid());
			if(!setjmp(p->pid_stage))
				longjmp(current->context, 1);
			
		}
		thread_spin_unlock(&lock);
		unblock_sigalrm();
		if(terminate_flag) {
			break;
		}	
	}
	return 0;
}
// wait till the job is completed and then return the value returned using return statement or thread_exit
int thread_join(thread_t thread, void **np) {
	int tid = thread.tid;
	mtm_thread_node *temp;

	/*Checking again and again if given thread id is in completed queue*/
	while(1) {
		thread_spin_lock(&lock);
		//block_sigalrm();
		temp = queueRemoveUsingThreadID(&completed, tid);
		//unblock_sigalrm();
		thread_spin_unlock(&lock);
		if(temp != NULL)
			break;		
	}

	/*Setting the return value once block is obtained from completed queue*/
	if(np != NULL)
		*np = temp->retval;

	block_sigalrm();
	/*Stopping timer and stopping all kernel threads if all user threads are done executing*/
	if(isempty(ready) && isempty(taken) && isempty(completed) && current == NULL) {
		/*Terminating all kernel threads*/
		terminate_flag = 1;
		for(int i = 0; i < kernel_thread_count; i++) {
			waitpid(process_array[i].pid, NULL, 0);
		}
		thread_count = 0;	
	}
	unblock_sigalrm();
	return 0;
}
