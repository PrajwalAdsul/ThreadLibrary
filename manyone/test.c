#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


void *thread(void *arg)
{
	int i = 0;
	while(i < 10000){
		printf("%d\n", i);
		i += 1;
		if(i == 100)
			sleep(1);
	}
	return NULL;
}
void* fun(void *args){
	printf("Hello World!\n");
}
int main(int argc, char *argv[])
{
	pthread_t tid;

	if (pthread_create(&tid, NULL, thread, NULL) != 0) {
		perror("pthread_create");
		exit(1);
	}
	sleep(1);
	pthread_kill(tid, SIGKILL);		
	pthread_join(tid, NULL);
	pthread_create(&tid, NULL, fun, NULL);
	pthread_join(tid, NULL);

	return 0;

}