#include "threads.h"
#include <stdio.h>
#include <unistd.h>
int a[10];
void* fun1(void *args){
	for(int i = 0; i < 5; i++)
		a[i] = 100;
}	
void* fun2(void *args){
	for(int i = 5; i < 10; i++)
		a[i] = 100;
}
int main()
{
	init();
	thread_t t1, t2;
	thread_create( &t1, fun1 , NULL);
	thread_create( &t2, fun2 , NULL);	
	void* n;
	thread_join(t1, &n);
	thread_join(t2, &n);
	for(int i = 0; i < 10; i++){
		printf("%d ", a[i]);
	}
	return 0;
}
