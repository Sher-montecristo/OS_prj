#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256

#define PHYSICAL_PAGE_FRAMES 128

#define MEMORY_SIZE (FRAME_SIZE * PHYSICAL_PAGE_FRAMES)

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
int nextFreeFrame = 0;
int nextFreeTLB = 0;

int total_cnt,page_fault_cnt,TLB_hit_cnt;
FILE *backing_storage,*input_file;

//function declaretion
int la2pa(int la);
int checkTLB(int page_number, int *frame_number);
void update_TLB(int page_number, int frame_number, clock_t tmp_timestamp);
void pageFaultHandler(int page_number);
int select_victim_frame();

int main(int argc, char *argv[]){
	//init
	if(argc != 2){
		printf("Incorrect number of arguments! \n");
		return 0;
	}
	input_file = fopen(argv[1], "r");
	if(input_file == NULL){
		printf("Open Failed! File: %s \n", argv[1]);
		return 0;
	}
	backing_storage = fopen("BACKING_STORE.bin","rb");
	if(backing_storage == NULL){
		printf("Open Failed! File: BACKING_STORE.bin \n");
		return 0;
	}
	for(int i = 0;i<PAGE_TABLE_SIZE;++i){
		pageTable[i].valid = 0;
	}
	
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
	
	//statistics
	printf("Page fault rate: %.2f%%\tTLB hit rate:%.2f%%\n", (float)page_fault_cnt/total_cnt*100,(float)TLB_hit_cnt/total_cnt*100);
	//shutdown
	fclose(input_file);
	fclose(backing_storage);
	return 0;
}

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
