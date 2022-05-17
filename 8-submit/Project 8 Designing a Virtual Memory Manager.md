## Project 8: Designing a Virtual Memory Manager

白骐硕 518030910102



#### 任务概述

This project consists of writing a program that translates logical to physical addresses for a virtual address space of size 2^16^ = 65,536 bytes. Your program will read from a file containing logical addresses and, using a TLB and a page table, will translate each logical address to its corresponding physical address and output the value of the byte stored at the translated physical address. Your learning goal is to use simulation to understand the steps involved in translating logical to physical addresses. This will include resolving page faults using demand paging, managing a TLB , and implementing a page-replacement algorithm.



#### 具体实现

##### 数据结构和常量定义

本实验使用c语言进行编程，为了便于后续编程，需要先定义若干常量：

- TLB entries 的个数
- page table entries 个数
- 页表大小
- 页框大小
- 内存中的页框个数
- 内存大小

```c
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define PHYSICAL_PAGE_FRAMES 256
#define MEMORY_SIZE (FRAME_SIZE * PHYSICAL_PAGE_FRAMES)
```

下面介绍所定义的结构体，为了实现完整的功能，需要定义的数据结构有：

- TLB
- Page Table
- Memory

```c
typedef struct{
	int page,frame;
	clock_t timestamp;
}TLB_Entry;

TLB_Entry TLB[TLB_SIZE];

typedef struct{
	int valid;
	int frame;
	clock_t timestamp;
}Page_Table_Entry;

Page_Table_Entry pageTable[PAGE_TABLE_SIZE];
char Memory[MEMORY_SIZE];
```

其中，TLB entry和 page table entry 分别使用结构体定义。需要特殊说明的是，为了实现LRU的替换算法（包括TLB的替换和 page 替换），两者中的每一个条目都含有其时间戳的信息。该时间戳通过函数 clock() 在每次访问相应条目时进行更新。

##### main函数

main函数中的内容非为四部分：

- 初始化，包括：
  - 打开文件（如果打开失败，则报错）
  - 将page table 所有条目的 valid bit置为0
- 执行功能：
  - 循环读取 addresses.txt的每一行，获得逻辑地址
  - 将逻辑地址转换为物理地址
  - 根据物理地址到memory数组中寻找值。
  - 将上述信息，根据格式打印出来
- 计算 page fault rate 和 TLB hit rate，并打印
- 关闭打开的文件

下面，仅展示核心部分（执行功能）的代码：

```c
//read from addresses
	char line[10];
	while(fgets(line,10,input_file)){
		int logical, physical;
		int8_t value;
		sscanf(line, "%d", &logical);
		++total_cnt;
		physical = la2pa(logical);
		value = Memory[physical];
		printf("Virtual address: %d Physical address: %d Value: %d \n", logical, physical, value);
	}
```

所以，该实验的关建在于 la2pa()函数的编写（logical address to physical address)

##### la2pa()函数

该函数的功能在于将输入参数的逻辑地址，转换从物理地址返回。编写的逻辑根据下图：

![1](D:\BQS_Learn\操作系统（吴晨涛）\prj\8\1.jpg)

- 首先check TLB 是否hit ，如果hit则

  - 更新TLB的时间戳
  - 将时间戳同步到page table
  - 直接得到物理地址并返回

  否则向下执行

- check page table 的相应条目的 valid bit是否为1，如果为1 则

  - 根据page table 得到 页框号
  - 更新page table 的时间戳
  - 将page table 的该条目添加到TLB中，如有必要执行TLB替换算法

  否则向下执行

- 此时发生page fault，

  - 如果memory 中有可用的frame，则将该frame添加到 page table 该条目下
  - 否则，则根据TLB算法寻找victim frame
  - 然后从backing store中，将数据读出到memory 中的该frame中

la2pa()函数的源代码如下：

```c
int la2pa(int la){
	int page_number,page_offset,frame_number;
	page_number = (la>>8)& 0XFF;
	page_offset = la & 0XFF;
	if(!checkTLB(page_number,&frame_number)){
		//TLB miss
		if(!pageTable[page_number].valid){
			//page fault
			++page_fault_cnt;
			pageFaultHandler(page_number);
		}
		frame_number = pageTable[page_number].frame;
		pageTable[page_number].timestamp = clock();
		update_TLB(page_number,frame_number, pageTable[page_number].timestamp);
	}
	int pa;
	pa = (frame_number << 8)|page_offset;
	return pa;
}
```

根据上述执行逻辑，下面介绍check TLB()函数，其功能为检查TLB中是否有该条目，如果找到则返回1，且修改传入的frame_number （该参数以指针方式传入），否则返回0

```c
int checkTLB(int page_number, int *frame_number){
	int max_TLB_entry = (nextFreeTLB<TLB_SIZE?nextFreeTLB:TLB_SIZE);
	for(int i = 0; i<max_TLB_entry; ++i){
		if(TLB[i].page == page_number){
			*frame_number = TLB[i].frame;
			clock_t tmp = clock();
			TLB[i].timestamp = tmp;
			pageTable[page_number].timestamp = tmp;
			++TLB_hit_cnt;
			return 1;
		}
	}
	return 0;
}
```

如果TLB miss，则向页表寻找对应的frame numebr。下面介绍处理page fault 的函数

```c
void pageFaultHandler(int page_number){
	pageTable[page_number].frame = select_victim_frame();
	pageTable[page_number].valid = 1;
	pageTable[page_number].timestamp = clock();
	fseek(backing_storage, page_number * PAGE_SIZE, SEEK_SET);
	fread(Memory+pageTable[page_number].frame*PAGE_SIZE, sizeof(char), PAGE_SIZE,backing_storage);
}

int select_victim_frame(){
	if(nextFreeFrame < PHYSICAL_PAGE_FRAMES){
		return nextFreeFrame++;
	}
	
	clock_t min_time = clock();
	int min_index;
	for(int i = 0; i<PAGE_TABLE_SIZE;++i){
		if(pageTable[i].valid && pageTable[i].timestamp<min_time){
			min_time = pageTable[i].timestamp;
			min_index = i;
		}
	}
	pageTable[min_index].valid = 0;
	return pageTable[min_index].frame;
}
```

如上图所示，在处理page fault 时，先寻找 victim frame，然后更新页表，并将数据从 backing storage 中写入到 该 frame中。

在寻找 victim frame时，我们首先判断free frame是否用完，如果没用完则返回下一个空页框，否则根据LRU算法，寻找一个时间戳最小的frame进行替换。

最后，介绍在TLB miss之后，已经根据page table找到对应的frame之后需要做的最后一步操作——更新TLB：

```c
void update_TLB(int page_number, int frame_number, clock_t tmp_timestamp){
	if(nextFreeTLB < TLB_SIZE){
		TLB[nextFreeTLB].page = page_number;
		TLB[nextFreeTLB].frame = frame_number;
		TLB[nextFreeTLB].timestamp = tmp_timestamp;
		++nextFreeTLB;
		return;
	}
	clock_t min_time = clock();
	int min_index;
	for(int i = 0; i<TLB_SIZE;++i){
		if(TLB[i].timestamp < min_time){
			min_time = TLB[i].timestamp;
			min_index = i;
		}
	}
	TLB[min_index].page = page_number;
	TLB[min_index].frame = frame_number;
	TLB[min_index].timestamp = tmp_timestamp;
}
```

与page replacement的逻辑十分类似，首先看TLB中是否有空的条目可用，如果有则直接将新的TLB信息写入到该空白条目中，如果没有空条目，则寻找一个时间戳最小的TLB entry，替换其中的信息。



#### 实验结果

##### 256 pages, 256 frames

![2](D:\BQS_Learn\操作系统（吴晨涛）\prj\8\2.png)

##### 256 pages, 128 frames

![3](D:\BQS_Learn\操作系统（吴晨涛）\prj\8\3.png)

