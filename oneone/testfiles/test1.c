#include "threads.h"
#include <stdio.h>
#include <unistd.h>
void* fun(void *args){
	while(1){
		printf("Hello World!\n");
		int r = 1000;
		thread_exit(&r);
	}
}	
void* fun11(void *args){
	printf("Hi\n");
	int r = 999;
	return r;
}
int main()
{
	init();
	thread_t t1, t2;
	thread_create( &t1, fun , NULL);
	thread_create( &t2, fun11 , NULL);	
	void* n;
	print();
	thread_join(t1, &n);
	printf("\nretval from join of thread1 %d\n", *(int*)(n));
	thread_kill(t2, SIGINT);
	thread_join(t2, &n);
	printf("\nretval from join of thread2 %d\n", *(int*)(n));
	return 0;
}
