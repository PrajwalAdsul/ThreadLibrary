#include "threads.c"
#include <stdio.h>
#include <unistd.h>

void* fun1(void *args){
	int i = 0;
	while(i < 10){
		sleep(1);
		printf("Fun1\n");
		i++;	
	}
}

void* fun2(void *args){
	printf("Fun2\n");
}

void* fiber1(void *ags)
{
	int i;
	for ( i = 0; i < 5; ++ i )
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

int main()
{
	/* Initialize the thread library */

	thread_t t1;
	thread_t t2;
	thread_t t3;
	
	thread_create( &t1, fun1 , NULL);
	thread_create( &t2, squares , NULL);
	thread_create( &t3, fibonacci , NULL);
	//pthread_create( &t3, NULL, squares , NULL);

	thread_join(t1, NULL);
	thread_join(t2, NULL);
	thread_join(t3, NULL);
	
	/*printf("Hello world\n");
	sleep(2);
	printf("Hello again\n");
	int abc;
	for(abc = 0; abc < 15; abc++) {
		sleep(1);
		printf("numb : %d\n", abc);	
	}*/
	/*sleep(3);
	printf("\n\nchecking\n\n");

	thread_create( &t1, fun1 , NULL);
	thread_create( &t2, squares , NULL);
	thread_create( &t3, fibonacci , NULL);
	//pthread_create( &t3, NULL, squares , NULL);

	thread_join(t1, NULL);
	thread_join(t2, NULL);
	thread_join(t3, NULL);*/
	
	/* The program ends */
	return 0;
}
