#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct block{
	char *name;
	int begin;
	int end;
	int used;
	struct block *prev,*next;
}Block;

int maxMem;
Block *memoryHead;
Block *memoryTail;

//function declaretion
void initMemory();
void shutdownMemory(Block *head);
void displayState();
void release();
void mergeHole(Block *p);
void request();
void insert2Hole(char *requestName, int requestSize, Block* targetHole);
void compact();

int main(int argc, char *argv[]){
	//get the commandLine
	if(argc != 2){
		fprintf(stderr, "Usage: ./main X | Example: ./main 1048576 \n");
		return 0;
	}
	maxMem = atoi(argv[1]);
	//initialize the memory
	initMemory();
	
	char op[10];
	printf("allocator>");
	while(scanf("%s", op) == 1){
		if(strcmp(op,"RQ")==0){
			//RQ
			request();
		}else if(strcmp(op, "RL")==0){
			//RL
			release();
		}else if(strcmp(op, "C")==0){
			//C
			compact();
		}else if(strcmp(op, "STAT") == 0){
			//STAT
			displayState();
		}else if(strcmp(op, "X") == 0){
			//X
			shutdownMemory(memoryHead);
			break;
		}else{
			printf("Illegal Operation! \n");
		}
		printf("allocator>");
	}
	
	return 0;
}

void initMemory(){
	memoryHead = malloc(sizeof(Block));
	memoryHead->begin = 0;
	memoryHead->end = maxMem - 1;
	memoryHead->used = 0;
	memoryHead->prev = NULL;
	memoryHead->next = NULL;
	memoryTail = memoryHead;
}

void shutdownMemory(Block *head){
	Block *p = head;
	Block *tmp = NULL;
	while(p!=NULL){
		tmp = p;
		p = p->next;
		if(tmp->used == 1){
			free(tmp->name);
		}
		free(tmp);
	}
}

void displayState(){
	printf("==================================================\n");
	Block *p = memoryHead;
	while(p!=NULL){
		if(p->used == 1){
			//Process
			printf("Addresses [%d:%d] Process %s \n", p->begin, p->end, p->name);
		}else{
			///Unused
			printf("Addresses [%d:%d] Unused \n", p->begin, p->end);
		}
		p = p->next;
	}
	printf("==================================================\n");
}

void release(){
	char releaseName[10];
	scanf("%s", releaseName);
	Block *p = memoryHead;
	int success = 0;
	while(p!=NULL){
		if(p->used == 0){
			p = p->next;
			continue;
		}
		if(strcmp(p->name, releaseName) == 0){
			success = 1;
			free(p->name);
			p->used = 0;
			//combine the adjacent hole
			mergeHole(p);
			break;
		}
		p = p->next;
	}
	if(success == 1){
		printf("Release Success! \n");
	}else{
		printf("No process named %s \nRelease Failed! \n", releaseName);
	}
}

void mergeHole(Block *p){
	int prevHole = 0;
	int nextHole = 0;
	//prev
	if(p->prev != NULL && p->prev->used == 0){
		prevHole = 1;
	}
	//next
	if(p->next != NULL && p->next->used == 0){
		nextHole = 1;
	}
	if(prevHole == 1 && nextHole == 1){
		Block *tmp = p->prev;
		tmp->end = p->end;
		tmp->next = p->next;
		p->next->prev = tmp;
		free(p);
		p = tmp->next;
		if(p->next ==NULL){
			tmp->end = p->end;
			tmp->next = NULL;
			memoryTail = tmp;
			free(p);
		}else{
			tmp->end = p->end;
			tmp->next = p->next;
			p->next->prev = tmp;
			free(p);
		}
	}
	if(prevHole == 1 && nextHole == 0){
		Block *tmp = p->prev;
		tmp->end = p->end;
		tmp->next = p->next;
		if(p->next!=NULL){
			p->next->prev = tmp;
		}else{
			memoryTail = tmp;
		}
		free(p);
	}
	if(prevHole == 0 && nextHole == 1){
		Block *tmp = p;
		p = p->next;
		tmp->end = p->end;
		tmp->next = p->next;
		if(p->next != NULL){
			p->next->prev = tmp;
		}else{
			memoryTail = tmp;
		}
		free(p);
	}
}

void request(){
	char requestName[10];
	char strategy;
	int requestSize;
	scanf("%s %d %c", requestName, &requestSize, &strategy);
	
	//check input
	if(requestSize <= 0){
		printf("Illegal Size! \n");
		return;
	}
	if(strategy != 'F' && strategy != 'B' && strategy != 'W'){
		printf("Illegal Strategy! \n");
		return;
	}
	
	Block *targetHole = NULL;
	switch(strategy){
		case 'F':
		{
			//first fit
			Block *p = memoryHead;
			while(p!=NULL){
				if(p->used == 1){
					p = p->next;
					continue;
				}
				if(p->end - p->begin +1 >= requestSize){
					targetHole = p;
					break;
				}
				p = p->next;
			}
			break;
		}
		case 'B':
		{
			//best fit
			Block *p = memoryHead;
			int minSize = maxMem+1;
			while(p!=NULL){
				if(p->used == 1){
					p = p->next;
					continue;
				}
				int tmpSize = p->end - p->begin + 1;
				if(tmpSize >= requestSize && tmpSize < minSize){
					targetHole = p;
					minSize = tmpSize;
				}
				p = p->next;
			}
			break;
		}
		case 'W':
		{
			//worst fit
			Block *p = memoryHead;
			int maxSize = 0;
			while(p!=NULL){
				if(p->used == 1){
					p = p->next;
					continue;
				}
				int tmpSize = p->end - p->begin + 1;
				if(tmpSize >= requestSize && tmpSize > maxSize){
					targetHole = p;
					maxSize = tmpSize;
				}
				p = p->next;
			}
			break;
		}
	}
	if(targetHole == NULL){
		printf("No enough memory hole to satisfy the request! \nRequest Denied! \n");
	}else{
		insert2Hole(requestName, requestSize, targetHole);
		printf("Request Success! \n");
	}
}

void insert2Hole(char *requestName, int requestSize, Block* targetHole){
	int targetHoleSize = targetHole->end - targetHole->begin + 1;
	if(targetHoleSize == requestSize){
		targetHole->name = malloc(sizeof(char) * (strlen(requestName)+1));
		strcpy(targetHole->name, requestName);
		targetHole->used = 1;
	}else{
		Block *tmp = malloc(sizeof(Block));
		tmp->name = malloc(sizeof(char) * (strlen(requestName)+1));
		strcpy(tmp->name, requestName);
		tmp->begin = targetHole->begin;
		tmp->end = tmp->begin + requestSize -1;
		tmp->used = 1;
		targetHole->begin = tmp->end +1;
		tmp->next = targetHole;
		tmp->prev = targetHole->prev;
		if(targetHole->prev == NULL){
			memoryHead = tmp;
			targetHole->prev = tmp;
		}else{
			tmp->prev->next = tmp;
			targetHole->prev = tmp;
		}
	}
}

void compact(){
	int usedEnd = -1;
	Block *newMemoryHead = NULL;
	Block *p = memoryHead;
	Block *last = NULL;
	while(p!=NULL){
		if(p->used == 0){
			p = p->next;
			continue;
		}
		Block *tmp = malloc(sizeof(Block));
		tmp->name = malloc(sizeof(char) * (strlen(p->name)+1));
		strcpy(tmp->name,p->name);
		tmp->begin = usedEnd+1;
		tmp->end = p->end - p->begin + tmp->begin;
		tmp->used = 1;
		usedEnd = tmp->end;
		//insert to new memory list
		if(last==NULL){
			newMemoryHead = tmp;
			tmp->next = NULL;
			tmp->prev = NULL;
			last = tmp;
		}else{
			last->next = tmp;
			tmp->prev = last;
			tmp->next = NULL;
			last = tmp;
		}
		p = p->next;
	}
	//insert the hole to the tail
	if(usedEnd+1 != maxMem){
		Block *tmp = malloc(sizeof(Block));
		tmp->begin = usedEnd+1;
		tmp->end = maxMem-1;
		tmp->used = 0;
		if(last==NULL){
			newMemoryHead = tmp;
			tmp->next = NULL;
			tmp->prev = NULL;
			last = tmp;
		}else{
			last->next = tmp;
			tmp->prev = last;
			tmp->next = NULL;
			last = tmp;
		}
	}
	shutdownMemory(memoryHead);
	memoryHead = newMemoryHead;
	memoryTail = last;
	printf("Compact Success! \n");
}
