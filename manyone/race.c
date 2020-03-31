/* A code demonstrating the race problem
*/
#include "threads.c"
#include <stdio.h>
#include<unistd.h>

//thread_spinlock_t lock;

long c = 0, c1 = 0, c2 = 0, run = 1;
void *thread1(void *arg) {
	printf("In thread1\n");
	while(run == 1) {
		c++;
		c1++;
	}
}
void *thread2(void *arg) {
	printf("In thread2\n");
	while(run == 1) {
		c++;
		c2++;
	}
}
int main() {
	thread_t th1, th2;
	int x;
	//sleep(2);
	//thread_spin_init(&lock); 
	thread_create(&th1, thread1, NULL);
	thread_create(&th2, thread2, NULL);
	block_sigalrm();
	x = sleep(2);
	unblock_sigalrm();
	run = 0;
	thread_join(th1, NULL);
	thread_join(th2, NULL);
	fprintf(stdout, "c = %ld c1+c2 = %ld c1 = %ld c2 = %ld timer : %d\n", c, c1+c2, c1, c2, x);
	fflush(stdout);
	//while(1);
}
