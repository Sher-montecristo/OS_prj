#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "buffer.h"

int main(int argc, char *argv[]) {
    // Get the command line arguments
    if(argc != 4) {
        fprintf(stderr,"Usage: <executable> sleepSecond numProducer numConsumer \n");
    }
    int sleep_time = atoi(argv[1]);
    int num_producer = atoi(argv[2]), num_consumer = atoi(argv[3]);

    // Initialize buffer
    buffer_init();

    // Create producer threads
    pthread_t *producers = malloc(num_producer * sizeof(pthread_t));
    int producer_id[num_producer];
    for (int i = 0; i < num_producer; ++i) {
        producer_id[i] = i+1;
        pthread_create(&producers[i],NULL,producer,&producer_id[i]);
    }

    // Create consumer threads
    pthread_t *consumers = malloc(num_consumer * sizeof(pthread_t));
    int consumer_id[num_consumer];
    for (int i = 0; i < num_producer; ++i) {
        consumer_id[i] = i+1;
        pthread_create(&consumers[i],NULL,consumer,&consumer_id[i]);
    }

    // Sleep
    printf("Sleep for %d second(s) before exit.\n", sleep_time);
    sleep(sleep_time);

    // Exit
	// cancel producer
    for(int i = 0; i != num_producer; ++i) {
        pthread_cancel(producers[i]);
        pthread_join(producers[i], NULL);
    }
    //cancel consumer
    for(int i = 0; i != num_consumer; ++i) {
        pthread_cancel(consumers[i]);
        pthread_join(consumers[i], NULL);
    }
    free(producers);
    free(consumers);
    buffer_shutdown();

    return 0;
}

