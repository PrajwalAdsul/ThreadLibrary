#include "threads.h"
#include <stdio.h>

thread_spinlock_t lock;

long c = 0, c1 = 0, c2 = 0, run = 1;
void *thread1(void *arg) {
	while(run == 1) {
		thread_spin_lock(&lock);
		c++;
		thread_spin_unlock(&lock);
		c1++;
	}
}
void *thread2(void *arg) {
	while(run == 1) {
		thread_spin_lock(&lock);
		c++;
		thread_spin_unlock(&lock);
		c2++;
	}
}
int main() {
	thread_t th1, th2;
	thread_spin_init(&lock); 
	thread_create(&th1, thread1, NULL);
	thread_create(&th2, thread2, NULL);
	sleep(2);
	run = 0;
	thread_join(th1, NULL);
	thread_join(th2, NULL);
	fprintf(stdout, "c = %ld c1+c2 = %ld c1 = %ld c2 = %ld \n", c, c1+c2, c1, c2);
	fflush(stdout);
}
