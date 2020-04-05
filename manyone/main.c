#include "threads1.c"
#include <stdio.h>
#include <unistd.h>
#include <time.h>       
#include <sys/types.h>

void* fun1(void *args){
	int i = 0;
	while(i < 10){
		// sleep(1);
		fprintf(stdout, "Fun1 11111\n");
		i++;	
		// thread_exit(NULL);
		// sleep(10);
	}
}

void* fun2(void *args){
	int i = 0;
	while(i < 50){
		// sleep(1);
		// printf("Fun1 11111\n");
		// printf("fun2\n");
		i++;	
		// thread_exit(&r);
		// sleep(10);
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
		fprintf(stdout, "fibonacci(%d) = %d\n", i, nextFib );
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
		fprintf(stdout, "%d*%d = %d\n", i, i, i*i );
		// sleep(1);
	}
}

int main()
{
	/* Initialize the thread library */

	thread_t t1;
	thread_t t2;
	thread_t t3;
	
	// thread_create( &t1, fun1 , NULL);
	//pthread_create( &t3, NULL, squares , NULL);
	// void *n;
	
	// thread_create( &t2, fibonacci , NULL);
	// thread_join(t1, NULL);
	// thread_create( &t3, fibonacci , NULL);
	

	// printf("\nbefore join\n");
	// sleep(1);
	// thread_join(t1, &n);
	// printf("\n&\n");
	
	// printf("hi\n");
	// fprintf(stdout, "join in t1 %d\n", *(int*)(n));
	// printf("\n&\n");
			
	// thread_join(t1, NULL);
	// thread_join(t2, NULL);
	// thread_join(t3, NULL);
	
	/*printf("Hello world\n");
	sleep(2);
	printf("Hello again\n");
	int abc;
	for(abc = 0; abc < 15; abc++) {
		sleep(1);
		printf("numb : %d\n", abc);	
	}*/
	// sleep(3);
	// printf("\n\nchecking\n\n");

	thread_create( &t1, fun1, NULL);
	thread_create( &t2, squares, NULL);
	thread_create( &t3, fibonacci, NULL);

	thread_kill(t1, SIGSTOP);
	//pthread_create( &t3, NULL, squares , NULL);

	// thread_join(t1, NULL);
	// thread_join(t2, NULL);
	// thread_join(t3, NULL);
	
	/* The program ends */
	return 0;
}
