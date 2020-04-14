#include "threads.h"
#include <stdio.h>
#include <unistd.h>

void* fun1(void *args){
	int i = 0;
	while(i < 5){
		printf("Fun1\n");
		i++;	
	}
}

void* fun2(void *args){
	int i = 0;
	while(i < 8){
		printf("Fun2\n");
		i++;	
	}
}

void* fiber1(void *ags)
{
	int i;
	for ( i = 0; i < 6; ++ i )
	{
		printf( "Hey, I'm thread #1: %d\n", i );
	}
}

void* fibonacci(void *ags)
{
	int i;
	int fib[2] = { 0, 1 };
	printf( "fibonacci(0) = 0\nfibonnaci(1) = 1\n" );
	for( i = 2; i < 15; ++ i )
	{
		int nextFib = fib[0] + fib[1];
		printf( "fibonacci(%d) = %d\n", i, nextFib );
		fib[0] = fib[1];
		fib[1] = nextFib;
	}
}

void* squares(void *ags)
{
	int i;
	
	for ( i = 0; i < 10; ++ i )
	{
		printf( "%d*%d = %d\n", i, i, i*i );
	}
}

void* fun3(void *ags)
{
	int i;
	
	for ( i = 0; i < 10; ++ i )
	{
		printf("fun3\n");
	}
}

int main()
{
	thread_t t1, t2, t3, t4, t5;
	thread_create( &t1, fun1 , NULL);
	thread_create( &t2, squares , NULL);
	thread_create( &t3, fun3 , NULL);
	thread_create( &t4, fun2 , NULL);
	thread_create( &t5, fiber1 , NULL);
	thread_join(t1, NULL);
	thread_join(t2, NULL);
	thread_join(t3, NULL);
	thread_join(t4, NULL);
	thread_join(t5, NULL);
	return 0;
}
