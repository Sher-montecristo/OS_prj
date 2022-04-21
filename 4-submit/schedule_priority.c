#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "schedulers.h"
#include "task.h"
#include "cpu.h"
#include "list.h"

#define MAX_NUM 100

struct node *taskList = NULL;

char *task_name[MAX_NUM];
int turnaround_time[MAX_NUM]={0},waiting_time[MAX_NUM]={0},response_time[MAX_NUM]={0};
int finish[MAX_NUM]={0};
int num=0;

void add(char *name, int priority, int burst){
    Task *t = malloc(sizeof(Task));
    t->name = malloc(sizeof(char)*(strlen(name)+1));
    strcpy(t->name,name);
    t->priority = priority;
    t->burst = burst;
    __sync_fetch_and_add(&num,1);
    t->tid = num;
    task_name[t->tid] = malloc(sizeof(char)*(strlen(t->name)+1));
    strcpy(task_name[t->tid],t->name);
    insert(&taskList,t);
}

// pickNextTask: pick the next task to execute with PRIORITY
//
// pick the task with the highest priority
Task *pickNextTask() {
    Task *highest_priority_job = taskList->task;
    struct node *p = taskList;
    while(p){
        if(p->task->priority >= highest_priority_job->priority){
            highest_priority_job = p->task;
        }
        p = p->next;
    }
    return highest_priority_job;
}

// invoke the scheduler
void schedule() {
	printf("Original Tasks List:\n");
	traverse(taskList);
	printf("\nRunning:\n");
    while(taskList) {
        Task *t = pickNextTask();
        run(t, t->burst);
        delete(&taskList, t);
        finish[t->tid]=1;
		
        // turnaround
        turnaround_time[t->tid]+=t->burst;
        for (int i = 1; i <= num; ++i) {
            if(!finish[i])
            {
                turnaround_time[i]+=t->burst;
            }
        }
        // waiting time
        for (int i = 1; i <= num; ++i) {
            if(!finish[i])
            {
                waiting_time[i]+=t->burst;
            }
        }
        // response time 
        for (int i = 1; i <= num; ++i) {
            if(!finish[i])
            {
                response_time[i]+=t->burst;
            }
        }
    }

    printf("\n");
    // print time
	printf("\nTime table:\n");
    double ave_res=0,ave_wait=0,ave_turn=0;
    for (int j = 1; j <= num; ++j) {
        printf("Task = %s , turnaround time = %d, waiting time = %d, response time = %d.\n",
               task_name[j],turnaround_time[j],waiting_time[j],response_time[j]);
        ave_res += response_time[j];
        ave_turn += turnaround_time[j];
        ave_wait += waiting_time[j];
    }
    ave_wait /= num;
    ave_res /= num;
    ave_turn /= num;
    printf("The average turnaround time = %.3f, waiting time = %.3f, response time = %.3f \n", ave_turn,ave_wait,ave_res);
}
