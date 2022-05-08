#ifndef PROJECT_5_2_BUFFER_H
#define PROJECT_5_2_BUFFER_H

#define BUFFER_SIZE 5
#define MAX_SLEEP_TIME 3

#define MAX_ITEM 100

typedef int buffer_item ;

int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void *consumer(void *param);
void *producer(void *param);
void buffer_init();
void buffer_shutdown();

#endif //PROJECT_5_2_BUFFER_H
