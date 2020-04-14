#include "threads.h"
#include <stdio.h>
#include <unistd.h>
void* fun(void *args){
	int i = 0;
	while(i < 5){
		write(1, "Hello World!\n", 13);
		int r = 1000;
		thread_exit(&r);
		i += 1;
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
	return 0;
}
