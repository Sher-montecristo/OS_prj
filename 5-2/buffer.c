#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "buffer.h"

//buffer
buffer_item buffer[BUFFER_SIZE];
int bufferHead,bufferTail;

//semaphore
sem_t full,empty;
//mutex
pthread_mutex_t lock;


int insert_item(buffer_item item) {
    sem_wait(&empty);
    pthread_mutex_lock(&lock);
    buffer[bufferTail] = item;
    bufferTail = (bufferTail + 1) %(BUFFER_SIZE);
    pthread_mutex_unlock(&lock);
    sem_post(&full);
    return 0;
}

int remove_item(buffer_item *item) {
    sem_wait(&full);
    pthread_mutex_lock(&lock);
    *item = buffer[bufferHead];
    bufferHead = (bufferHead+1) % BUFFER_SIZE;
    pthread_mutex_unlock(&lock);
    sem_post(&empty);
    return 0;
}


void *producer(void *param) {
    buffer_item item;
    while (1) {
        sleep(rand()% MAX_SLEEP_TIME+1);
        item = rand() % MAX_ITEM;
        if(insert_item(item)) {
            fprintf(stderr,"Error: produce failed!");
        } else {
            printf("The Producer %d produces the value %d. \n" , *(int*) param, item);
        }
    }
}


void *consumer(void *param) {
    buffer_item item;
    while(1) {
        sleep(rand()% MAX_SLEEP_TIME+1);
        if(remove_item(&item)) {
            fprintf(stderr,"Error: consume failed!");
        } else {
            printf("The Consumer %d consumes the value %d. --Consumer\n" , *(int*) param, item);
        }
    }
}

//init
void buffer_init() {
    pthread_mutex_init(&lock,NULL);
    sem_init(&full,0,0);
    sem_init(&empty,0,BUFFER_SIZE);
    bufferHead = 0;
    bufferTail = 0;
}

//destroy
void buffer_shutdown() {
    sem_destroy(&full);
    sem_destroy(&empty);
    pthread_mutex_destroy(&lock);
}
