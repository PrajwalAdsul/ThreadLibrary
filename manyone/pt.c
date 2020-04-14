#include <stdio.h>
#include <pthread.h>
void* fun(void* args){

}
int main(){
	pthread_t t1;
	pthread_create(&t1, NULL, fun, NULL);
	int n;
	pthread_join(t1, (void**)&n);
	printf("join -> %d\n", n);
	return 1;
}