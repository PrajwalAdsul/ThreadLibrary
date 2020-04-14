#include "threads.h"
#include <stdio.h>
#include <unistd.h>
void* fun(void *args){
	int i = 0;
	while(i < 10){
		printf("Hello World!\n");
		int r = 100;
		thread_exit(&r);
		i++;
	}
}	
void* fun11(void *args){
	printf("Hi\n");
	int r = 999;
	return r;
}
int main()
{
	thread_t t1, t2;
	thread_create( &t1, fun , NULL);
	thread_create( &t2, fun11 , NULL);	
	void* n;
	thread_kill(t1, SIGCONT);
	thread_join(t1, &n);
	thread_join(t2, &n);
	printf("\njoin is %d\n", *(int*)n);
	return 0;
}