/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

// the work queue
task worktodoQueue[QUEUE_SIZE+1]; //end is always empty
int queueBegin = 0, queueEnd = 0;

// the worker bee
pthread_t bees[NUMBER_OF_THREADS];

//mutex for enqueue and dequeue
pthread_mutex_t lock;
//semaphore
sem_t taskNum;

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
	pthread_mutex_lock(&lock);
    if((queueEnd + 1)%QUEUE_SIZE == queueBegin){
    	//full
    	pthread_mutex_unlock(&lock);
    	return 1;
    }else{
		worktodoQueue[queueEnd] = t;
		queueEnd = (queueEnd + 1)%QUEUE_SIZE;
		pthread_mutex_unlock(&lock);
		return 0;
    }
    return 0;
}

// remove a task from the queue
task dequeue() 
{
	pthread_mutex_lock(&lock);
	task tmp = worktodoQueue[queueBegin];
	queueBegin = (queueBegin+1)%QUEUE_SIZE;
	pthread_mutex_unlock(&lock);
    return tmp;
}

// the worker thread in the thread pool
void *worker(void *param)
{
	task tmp;
	while(TRUE){
		//execute the task
		sem_wait(&taskNum);
		tmp = dequeue();
		execute(tmp.function, tmp.data);
	}	
    //pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
	int flag;
	task newTask;
	newTask.function = somefunction;
    newTask.data = p;
	flag = enqueue(newTask);
	if(!flag){
		//enqueue success
		sem_post(&taskNum);
	}
    return flag;
}

// initialize the thread pool
void pool_init(void)
{
	//mutex
	pthread_mutex_init(&lock,NULL);
	//semaphore
	sem_init(&taskNum,0,0);
	//thread create
	for(int i=0;i<NUMBER_OF_THREADS;++i){
		pthread_create(&bees[i],NULL,worker,NULL);
	}
}

// shutdown the thread pool
void pool_shutdown(void)
{
	//thread cancel
	for(int i = 0; i<NUMBER_OF_THREADS;++i){
		pthread_cancel(bees[i]);
		pthread_join(bees[i], NULL);
	}
	//semaphore
	sem_destroy(&taskNum);
	//mutex
	pthread_mutex_destroy(&lock);
}
