#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4

//available
int available[NUMBER_OF_RESOURCES];

//maximum
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

//allocation
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

//need
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

//functions declaraction
int get_commandline(int argc, char *argv[]);
int init_maximum();
void print_usage();
void display_matrix();
void request();
void release();
int request_resources(int customer_num, int request[]);
void release_resources(int customer_num, int release[]);
int check_safe();
int is_leq_matrix(int *a, int *b, int length);

int main(int argc, char *argv[]){
	//get commandline input and initialize the avaliable
	if(get_commandline(argc,argv)!=0){
		return 0;
	}
	if(init_maximum() != 0){
		return 0;
	}
	printf("Initialization Success! \n");
	
	char command[10];
	printf(">>");
	while(scanf("%s", command) == 1){
		if(strcmp(command, "RQ") == 0){
			//RQ
			request();
		}else if(strcmp(command, "RL") == 0){
			//RL
			release();
		}else if(strcmp(command, "exit") == 0){
			//exit
			break;
		}else if(strcmp(command, "*") == 0){
			//display matrix
			display_matrix();
		}else{
			//Illegal input
			print_usage();
		}
		printf(">>");
	}
	return 0;
}


int get_commandline(int argc, char *argv[]){
	if(argc != NUMBER_OF_RESOURCES+1){
		fprintf(stderr, "Incorrect input. Resource number should be %d \n", NUMBER_OF_RESOURCES);
		return -1;
	}
	for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
		available[i] = atoi(argv[i+1]);
	}
	return 0;
}

int init_maximum(){
	FILE *f = fopen("maximum_init.txt", "r");
	if(f == NULL){
		fprintf(stderr, "Failed to open maximum_init.txt \n");
		return -1;
	}
	for(int i = 0; i < NUMBER_OF_CUSTOMERS; ++i){
		for(int j = 0; j < NUMBER_OF_RESOURCES; ++j){
			fscanf(f, "%d", &maximum[i][j]);
			need[i][j] = maximum[i][j];
		}
	}
	fclose(f);
	return 0;
}

void print_usage(){
    printf("=======================================\n");
    printf("Command: \n");
    printf("Request resources: RQ X X X X.\n");
    printf("Release resources: RL X X X X.\n");
    printf("Output the value: *\n");
    printf("Exit: exit \n");
    printf("=======================================\n");
}

void display_matrix(){
	printf("=============================================================\n");
	printf("Available resources:\n");
    for(int i = 0; i != NUMBER_OF_RESOURCES; ++i) {
        printf("%d ", available[i]);
    }
    printf("\n");
    printf("-------------------------------------------------------------\n");
    printf("Maximum resources for each customer:\n");
    for(int customer = 0; customer != NUMBER_OF_CUSTOMERS; ++customer) {
        printf("Customer %d: ", customer);
        for(int r = 0; r != NUMBER_OF_RESOURCES; ++r) {
            printf("%d ", maximum[customer][r]);
        }
        printf("\n");
    }
    printf("-------------------------------------------------------------\n");
    printf("Allocated resources for each customer:\n");
    for(int customer = 0; customer != NUMBER_OF_CUSTOMERS; ++customer) {
        printf("Customer %d: ", customer);
        for(int r = 0; r != NUMBER_OF_RESOURCES; ++r) {
            printf("%d ", allocation[customer][r]);
        }
        printf("\n");
    }
    printf("-------------------------------------------------------------\n");
    printf("Needed resources for each customer:\n");
    for(int customer = 0; customer != NUMBER_OF_CUSTOMERS; ++customer) {
        printf("Customer %d: ", customer);
        for(int r = 0; r != NUMBER_OF_RESOURCES; ++r) {
            printf("%d ", need[customer][r]);
        }
        printf("\n");
    }
    printf("=============================================================\n");
}

void request(){
	int request[NUMBER_OF_RESOURCES];
	int customer_num;
	scanf("%d",&customer_num);
	for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
		scanf("%d", &request[i]);
	}
	
	//check input
	if(customer_num < 0 || customer_num >= NUMBER_OF_CUSTOMERS){
		printf("Illegal customer number! \n");
		printf("Request Denied! \n");
		return;
	}
	for(int i = 0; i<NUMBER_OF_RESOURCES; ++i){
		if(request[i] < 0){
			printf("For resource %d: request less than 0! \n", i);
			printf("Request Denied! \n");
			return;
		}
		if(request[i] > need[customer_num][i]){
			printf("For resource %d: request %d, but need is %d \n", i, request[i], need[customer_num][i]);
			printf("Request Denied! \n");
			return;
		}
		if(request[i] > available[i]){
			printf("For resource %d: request %d, but available is %d \n", i, request[i], available[i]);
			printf("Request Denied! \n");
			return;
		}
	}
	if(request_resources(customer_num, request) != 0){
		printf("Request Denied! \n");
	}else{
		printf("Request Success! \n");
	}
}

int request_resources(int customer_num, int request[]){
	//allocate
	for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
		available[i] -= request[i];
		allocation[customer_num][i] += request[i];
		need[customer_num][i] -= request[i];
	}
	if(!check_safe()){
		//recover
		printf("Request will result in a unsafe state. \n");
		for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
			available[i] += request[i];
			allocation[customer_num][i] -= request[i];
			need[customer_num][i] += request[i];
		}
		return -1;
	}
	return 0;
}

int check_safe(){
	int Work[NUMBER_OF_RESOURCES],Finish[NUMBER_OF_CUSTOMERS];
    memcpy(Work,available,NUMBER_OF_RESOURCES*sizeof(int));
    memset(Finish,0,NUMBER_OF_CUSTOMERS*sizeof(int));
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        int flag = 0;
        for (int j = 0; j < NUMBER_OF_CUSTOMERS; ++j) {
            if(!Finish[j] && is_leq_matrix(need[j],Work, NUMBER_OF_RESOURCES)) {
                flag = 1;
                Finish[j] = 1;
                for (int k = 0; k < NUMBER_OF_RESOURCES; ++k) {
                    Work[k] += allocation[j][k];
                }
                break;
            }
        }
        if(!flag) {
            return  0;
        }
    }
    return 1;
}

int is_leq_matrix(int *a, int *b, int length){
	for(int i = 0; i<length; ++i){
		if(a[i] > b[i]){
			return 0;
		}
	}
	return 1;
}

void release(){
	int release[NUMBER_OF_RESOURCES];
	int customer_num;
	scanf("%d", &customer_num);
	for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
		scanf("%d", &release[i]);
	}
	//check input
	if(customer_num < 0 || customer_num >= NUMBER_OF_CUSTOMERS){
		printf("Illegal customer number! \n");
		printf("Release Denied! \n");
		return;
	}
	for(int i = 0; i<NUMBER_OF_RESOURCES; ++i){
		if(release[i] < 0){
			printf("For resource %d: release less than 0! \n", i);
			printf("Release Denied! \n");
			return;
		}
		if(release[i] > allocation[customer_num][i]){
			printf("For resource %d: release %d, but allocation is %d \n", i, release[i], allocation[customer_num][i]);
			printf("Release Denied! \n");
			return;
		}
	}
	release_resources(customer_num, release);
	printf("Release Success! \n");
}

void release_resources(int customer_num, int release[]){
	for(int i = 0; i < NUMBER_OF_RESOURCES; ++i){
		allocation[customer_num][i] -= release[i];
		available[i] += release[i];
		need[customer_num][i] += release[i];
	}
}
