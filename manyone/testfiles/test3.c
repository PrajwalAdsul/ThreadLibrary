#include "threads.h"
#include <stdio.h>
#include <unistd.h>

void* fact1(void *args){
	printf("value is %d\n", *(int*)args);
	return 1 * 2 * 3 * 4;

}

void* fact2(void *args){
	return 5 * 6 * 7;
}

void* fact3(void *args){
	return 8 * 9;
}

int main()
{
	thread_t t1, t2, t3;
	thread_create( &t1, fact1 , 1);
	thread_create( &t2, fact2 , NULL);
	thread_create( &t3, fact3 , NULL);		
	void *n1, *n2, *n3;
	thread_join(t1, &n1);
	thread_join(t2, &n2);
	thread_join(t3, &n3);
	printf("Factorial of 9 is %d\n", (*(int*)n1) * (*(int*)n2) * (*(int*)n3));
	return 0;
}