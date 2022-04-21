## Project 4: Scheduling Algorithms

白骐硕 518030910102



#### 任务概述

This project involves implementing several different process scheduling algorithms. The scheduler will be assigned a predefined set of tasks and will schedule the tasks based on the selected scheduling algorithm. Each task is assigned a priority and CPU burst. The following scheduling algorithms will
be implemented:

- First-come, first-served ( FCFS ), which schedules tasks in the order in which they request the CPU .
- Shortest-job-first ( SJF ), which schedules tasks in order of the length of the tasks’ next CPU burst.
- Priority scheduling, which schedules tasks based on priority.
- Round-robin ( RR ) scheduling, where each task is run for a time quantum (or for the remainder of its CPU burst).
- Priority with round-robin, which schedules tasks in order of priority and uses round-robin scheduling for tasks with equal priority.

Priorities range from 1 to 10, where a higher numeric value indicates a higher relative priority. For round-robin scheduling, the length of a time quantum is 10 milliseconds.



#### 具体实现

##### 概述

根据已有的代码框架，需要实现5个c语言文件：

- schedule_fcfs.c
- schedule_sjf.c
- schedule_priority.c
- schedule_rr.c
- schedule_priority_rr.c



每种schedule algorithm中需要实现三个函数：

- `add()` 用于向 taskList 中添加任务，由main函数（即driver.c）调用
- `pickNextTask()` 根据特定的调度算法从 taskList 中选出下一个执行的任务，在`schedule()` 中调用
- `schedule()` 执行任务调度代码的外部接口，由main函数（即driver.c）调用



每种schedule algorithm中都需要定义如下全局变量：

```c
#define MAX_NUM 100

struct node *taskList = NULL;

char *task_name[MAX_NUM];
int turnaround_time[MAX_NUM]={0},waiting_time[MAX_NUM]={0},response_time[MAX_NUM]={0};
int finish[MAX_NUM]={0};
int num=0;
```

其中，每个变量的用途如下：

- task_name[] 用于保存每个任务的任务名，为后续输出时间使用。下标为任务的 tid

- turnaround_time[] 用于记录每个任务的turnaround time。下标为任务的 tid

  waiting_time[] 和 response_time[] 同理。

- finish[] 用于记录每个任务是否执行完成。下标为任务的 tid

- num 记录任务的个数。在添加任务时，每增添一个任务，使用函数 `__sync_fetch_and_add(&num,1)` 将num增加1，并用当前num作为该任务的 tid。这样便完成了 Further Challenges 1 的要求。

特别的在 RR算法和 Priority RR 算法中。为了计算 respons time，还需要定义一个新的全局变量：

```c
int visit[MAX_NUM] = {0};
```

用于记录每个任务是否执行过（即response）。



##### FCFS

首先需要实现 add() 函数：

```c
void add(char *name, int priority, int burst) {
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
```

利用该函数的三个参数，创建一个 Task 类型的实例 t，之后将该任务使用 list.c 中的 insert()函数插入到 taskList中。同时，将该任务的name记录到字符串数组task_name[]中。

由于在本实验的要求中限制了所有的任务都是同时到达的，所以我们只需要按照 taskList的插入顺序进行调度即可。根据 insert() 函数的实现可知， insert() 函数每次插入的时候，将新的 task 插入到表头。所以，在选择下一个任务的时候，我们每次都选取链表尾部的task即可。代码如下：

```c
// pickNextTask: pick the next task to execute with FCFS
//
// pick the task at the end of the list
Task *pickNextTask() {
    struct node *lastNode = taskList;

    while(lastNode->next) {
        lastNode = lastNode->next;
    }
    return lastNode->task;
}
```

`schedule()` 函数执行逻辑为：当taskList不为空时，通过上述 `pickNextTask()` 函数选择出下一个进入执行的任务，然后调用 `run()` 执行该任务。最后，通过 `delete()` 函数将该任务从 taskList 中删除，并将该任务标记为已完成 `finish[t->tid] = 1`。

`schedule()` 函数的核心代码如下：

```c
void schedule(){
    while(taskList){
        Task *t = pickNextTask();
        run(t, t->burst);
        delete(&taskList, t);
        finish[t->tid] = 1;
    }
}
```

下面，我们需要考虑在 `schedule()` 函数中加入对 turnaround time，waiting time，response time 的计算。在每次循环中，即每次`run()`执行结束，我们都需要对三个记录时间的数组进行更新。对于FCFS算法：

- 所有未完成的任务和当前任务的 turnaround time 都需要加上当前任务的 burst time
- 所有未完成任务的 waiting time 和 response time 都需要加上当前任务的 burst time

完整的 `schedule()` 函数的实现如下：

```c
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
        printf("Task = %s , turnaround time = %d, waiting time = %d, response time = %d.\n", task_name[j],turnaround_time[j],waiting_time[j],response_time[j]);
        ave_res += response_time[j];
        ave_turn += turnaround_time[j];
        ave_wait += waiting_time[j];
    }
    ave_wait /= num;
    ave_res /= num;
    ave_turn /= num;
    printf("The average turnaround time = %.3f, waiting time = %.3f, response time = %.3f \n", ave_turn,ave_wait,ave_res);

}
```



##### SJF

SJF 算法的实现和 FCFS 算法的唯一区别在于 `pickNextTask()` 函数的不同。SJF 需要在该函数中实现选择出具有最小 burst time的任务进入下一次执行。

```c
// pickNextTask: pick the next task to execute with SJF
//
// pick the task with the shortest burst time
Task *pickNextTask() {
    Task *shortest_job = taskList->task;
    struct node *p = taskList;
    while(p){
        if(p->task->burst <= shortest_job->burst){
            shortest_job = p->task;
        }
        p = p->next;
    }
    return shortest_job;
}
```



##### Priority

Priority 算法的实现和FCFS 算法的唯一区别也只在于 `pickNextTask()` 函数的不同。Priority 算法需要每次选择出具有最高优先级的任务进入下一次执行。

```c
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
```



##### RR

`add()` 函数与之前的算法相同。

对于RR的调度，逻辑如下：

- 每次选择 taskList 中尾部的那个任务进入执行
- 执行时，需要判断当前任务的 burst time 是否大于 QUANTUM，如果大于则执行的slice = QUANTUM ，否则为 burst time
- 执行完毕后
  - 修改该任务的 burst time（`t->burst -= slice`）
  - 在visit[] 数组中将该任务标记为已经被访问过
  - 将该任务从 taskList 中删除
  - 如果该任务已经执行完毕，则在finish[] 数组中将标记为执行完毕。否则将其从新插入到 taskList 链表的头部

根据上述逻辑可知，`pickNextTask()` 函数与 FCFS相同，从链表的尾部取出一个任务。

关建在于 `schedule()` 函数的实现，代码如下：(对于 time 计算的部分省略)

```c
void schedule() {
    while(taskList) {
        Task *t = pickNextTask();
	int slice = 0;
	int done = 0;
	if((t->burst) > QUANTUM){
	    slice = QUANTUM;
	}else{
	    slice = t->burst;
	    done = 1;
	}
	visit[t->tid] = 1;
    run(t, slice);
    t->burst -=slice;
	delete(&taskList,t); //delete from the tail
	if(!done){
	    insert(&taskList,t); //insert to the head
	}else{
	    finish[t->tid] = 1;
	}
    }
}
```



##### Priority RR

为了实现 Priority RR，将原先的单个 taskList进行拓展，创建多个 taskList，每个 priority 对于一个 taskList。在调度时，从最高优先的 taskList开始执行，使用RR算法遍历所有 taskList 即可。

```c
// pickNextTask: pick the next task to execute with PRIORITY_RR
//
// pick the task at the end of the current list 
Task *pickNextTask(struct node *tl) {
    struct node *lastNode = tl;
    while(lastNode->next){
		lastNode = lastNode->next;
    }
    return lastNode->task;
}

void schedule() {
    //from the high priority to low priority
    for (int i = MAX_PRIORITY; i >= MIN_PRIORITY ; --i) {
        while (taskList[i]) {
            Task *t = pickNextTask(taskList[i]);
	    	int slice = 0;
	    	int done = 0;
	    	if((t->burst) > QUANTUM){
	    		slice = QUANTUM;
	    	}else{
	    		slice = t->burst;
	    		done = 1;
	    	}
	    	visit[t->tid] = 1;
            run(t, slice);
            t->burst -= slice;
            delete(&taskList[i],t); //delete from the tail
            if(!done){
            	insert(&taskList[i],t);  //insert to the head
            }else{
            	finish[t->tid] = 1;
            }
        }
    }
}
```



##### Further Challenges

1. 如前文所述，定义了一个全局变量 num (初值为0），在每次调用`add()`函数添加任务的时候，使用`__sync_fetch_and_add(&num, 1)` 将其增加1。用num变量作为任务的 tid。这样便避免了分配 tid 时的 race condition。具体代码参考前文所述，
2. 对于时间的计算，我们定义了三个全局数组，用于维护三种不同的 time。在调度的每个循环中，即cpu每次 `run()` 之后修改相关任务的 time 值。具体修改规则参考前文。



#### 实验结果

测试用例为
$$
\begin{align}
T1,\ 4,\ 20\\
T2,\ 3,\ 25\\
T3,\ 3,\ 25\\
T4,\ 5,\ 15\\
T5,\ 5,\ 20\\
T6,\ 1,\ 10\\
T7,\ 3,\ 30\\
T8,\ 10,\ 25\\
\end{align}
$$

##### FCFS

![1](D:\BQS_Learn\操作系统（吴晨涛）\prj\4\1.png)

##### SJF

![2](D:\BQS_Learn\操作系统（吴晨涛）\prj\4\2.png)

##### Priority

![3](D:\BQS_Learn\操作系统（吴晨涛）\prj\4\3.png)

##### RR

![4](D:\BQS_Learn\操作系统（吴晨涛）\prj\4\4.png)

##### Priority RR

![5](D:\BQS_Learn\操作系统（吴晨涛）\prj\4\5.png)

