/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
	int n = 0;
	while(n <= 0){
		printf("Please input the number of tasks:");
		scanf("%d",&n);
	}
    // create some work to do
    struct data *work = malloc(n * sizeof(struct data));
    //initialize a and b 
    for(int i = 0 ;i<n;++i){
    	work[i].a = rand()%100;
    	work[i].b = rand()%100;
    }

    // initialize the thread pool
    pool_init();

    // submit the work to the queue
    for(int i = 0; i<n;++i){
    	while(pool_submit(&add,&work[i]) != 0);
    }

	printf("submit done!\n");
    // may be helpful 
    //sleep(3);

    pool_shutdown();
	free(work);
	
    return 0;
}
