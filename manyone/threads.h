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
#define MAX_THREADS 50


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

int init_queue(queue *q);

int enqueue(queue *q, user_thread_node *temp);

user_thread_node* dequeue(queue *q);

user_thread_node* queueRemoveUsingID(queue *q, int id);

int isempty(queue q);

void printq(queue q);

void block_sigalrm();

void unblock_sigalrm();

void createStackForThreadandExecute(int signum);

int thread_exit(void *retval);

int invalidSigNo(int signo);

bool isThreadCompleted(int tid);

int thread_kill(thread_t thread, int signo);

void init_main_context();

void handle_sigalrm();

static bool init_timer();

int thread_create(thread_t *thread, void *(*start_routine) (void *), void *arg);


int thread_join(thread_t thread, void **np);