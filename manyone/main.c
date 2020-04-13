#include "threads.c"
#include <stdio.h>
#include <unistd.h>
#include <time.h>       
#include <sys/types.h>

void* fun1(void *args){
	int i = 0;
	while(i < 10){
		// fprintf(stdout, "Fun1 11111\n");
		write(1, "Fun1 11111\n", 11);
		i++;
		int r = 100;
		thread_exit(&r);	
	}
}

void* fun2(void *args){
	int i = 0;
	while(i < 50){
		i++;	
	}
}

void* fiber1(void *ags)
{
	int i;
	for ( i = 0; i < 3; ++ i )
	{
		fprintf(stdout, "Hey, I'm thread #1: %d\n", i );
	}
}

void* fibonacci(void *ags)
{
	int i;
	int fib[2] = { 0, 1 };
	
	fprintf(stdout, "fibonacci(0) = 0\nfibonnaci(1) = 1\n" );
	for( i = 2; i < 5; ++ i )
	{
		int nextFib = fib[0] + fib[1];
		// fprintf(stdout, "fibonacci(%d) = %d\n", i, nextFib );
		char buf[64];
		sprintf(buf, "fibonacci(%d) = %d\n", i, nextFib );
		write(1, buf, sizeof(buf));

		fib[0] = fib[1];
		fib[1] = nextFib;
		// thread_exit(NULL);
		// sleep(1);
	}
}

void* squares(void *ags)
{
	int i;
	
	for ( i = 0; i < 3; ++ i )
	{
		char buf[64];
		sprintf(buf, "%d*%d = %d\n", i, i, i*i );
		write(1, buf, sizeof(buf));
		// sleep(1);
	}
}

int main()
{
	thread_t t1;
	thread_t t2;
	thread_t t3;
	thread_create( &t1, fun1, NULL);
	thread_create( &t2, squares, NULL);
	thread_create( &t3, fibonacci, NULL);
	int retval;
	thread_join(t1, (void**)&retval);
	printf("\njoin => %d\n", retval);
	wait(NULL);
	return 0;
}
