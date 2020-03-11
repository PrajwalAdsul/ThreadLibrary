#include "threads.c"
#include <stdio.h>
#include <unistd.h>

void* fun1(void *args){
	int i = 0;
	while(i < 50){
	 printf("%d Fun1\n", i);
	 i++;	
	}
}

void* fun2(void *args){
	int i = 0;
	while(i < 50){
	 printf("%d Fun2\n", i);
	 i++;
	}
}

void* fiber1(void *ags)
{
	int i;
	for ( i = 0; i < 5; ++ i )
	{
	 printf( "Hey, I'm fiber #1: %d\n", i );
	}
}

void* fibonacci(void *ags)
{//nfc
	int i;
	int fib[2] = { 0, 1 };
	
	/*sleep( 2 ); */

	 fflush(stdin);
	 fflush(stdout);
	printf( "fibonacchi(0) = 0\nfibonnachi(1) = 1\n" );
	for( i = 2; i < 7; ++ i )
	{
		int nextFib = fib[0] + fib[1];
		printf( "fibonacchi(%d) = %d\n", i, nextFib );
		fib[0] = fib[1];
		fib[1] = nextFib;
	}
}

void* squares(void *ags)
{
	int i;
	
	/*sleep( 5 ); */
	for ( i = 0; i < 10; ++ i )
	{
		printf( "%d*%d = %d\n", i, i, i*i );
	}
}
void* fun3(void *args){
	while(1){
		printf("*\n");
	}
}

int main()
{
	/* Initialize the fiber library */
	//initFibers();
	thread_t t1;
	thread_t t2;
	thread_t t3;
	
	/* Go fibers! */
	thread_create( &t3, fibonacci , NULL);
	// kill(t3.pid, 9);

	thread_create( &t1, fun1 , NULL);
	thread_create( &t2, fun2 , NULL);
	
	/* Since these are nonpre-emptive, we must allow them to run */
	thread_join(t3);
	thread_join(t1);
	thread_join(t2);
	//pthread_join(t3, NULL);
	
	/* The program quits */
	return 0;
}
