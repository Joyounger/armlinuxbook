#include <stdio.h>
#include <pthread.h>

void thread(void)
{
	int i;
	for(i=0;i<3;i++)
		printf("This is a pthread.\n");
}

int main()
{
	pthread_t id;
	int i,ret;
	ret=pthread_create(&id,NULL,(void*)thread,NULL);
	if(ret!=0) {
		printf("Create pthread fail!\n");
		exit(1);
	}
	for(i=0;i<3;i++) {
		printf("This is the main process.\n");
	}
	pthread_join(id,NULL);
	
	return 0;
}

/* 
$ ./exampthread.exe
This is the main process.
This is a pthread.
This is the main process.
This is a pthread.
This is the main process.
This is a pthread.
 */
