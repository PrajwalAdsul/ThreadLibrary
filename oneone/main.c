#include "threads.c"
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
	// sleep(2);
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
	printf("\njoin %d\n", *(int*)(n));
	thread_kill(t2, SIGINT);
	thread_join(t2, &n);
	printf("\njoin %d\n", *(int*)(n));
	return 0;
}


// void* fun1(void *args){
// 	int i = 0;
// 	while(i < 10){
// 		// printf("Fun1\n");
	
// 		// Uncomment this to test thread_exit		
// 		if(i == 5){
// 			void *retval;
// 			thread_exit(&retval);
// 	 	}
// 	 	printf("%d Fun1\n", i);
// 	 	// printf("%d Fun1 %d\n", i, getpid());
// 	 i++;	
// 	}
// }

// void* fun2(void *args){
// 	int i = 0;
// 	while(i < 10){
// 		// printf("Fun2\n");
// 	 	printf("%d Fun2\n", i);
// 	 	// printf("%d Fun2 %d\n", i, getpid());
// 	 i++;
// 	}
// }

// void* fibonacci(void *ags)
// {//nfc
// 	int i;
// 	int fib[2] = { 0, 1 };
	
// 	sleep( 2 ); 

// 	 fflush(stdin);
// 	 fflush(stdout);
// 	printf( "fibonacchi(0) = 0\nfibonnachi(1) = 1\n" );
// 	for( i = 2; i < 7; ++ i )
// 	{
// 		int nextFib = fib[0] + fib[1];
// 		printf( "fibonacchi(%d) = %d\n", i, nextFib);
// 		// printf( "fibonacchi(%d) = %d %d\n", i, nextFib, getpid());
// 		fib[0] = fib[1];
// 		fib[1] = nextFib;
// 	}
// }

// void* squares(void *ags)
// {
// 	int i;
	
// 	/*sleep( 5 ); */
// 	for ( i = 0; i < 10; ++ i )
// 	{
// 		printf( "%d*%d = %d\n", i, i, i*i );
// 	}
// }
// void* fun3(void *args){
// 	while(1){
// 		printf("*\n");
// 	}
// }
