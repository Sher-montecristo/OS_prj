#include <stdio.h>
#include <pthread.h>

#define MAX_NUM  100

int array1[MAX_NUM];
int array2[MAX_NUM];
int array_size = 0;

void *bubbleSort(void *param);
void *merge(void *param);

typedef struct{
    int low;
    int high;
}para;



int main(){
    pthread_t tid[3];
    pthread_attr_t attr;
    
    //get input
    while(array_size<=1 || array_size>=100){
        printf("Input the number of elements(2~99): ");
        scanf("%d", &array_size);
    }
    printf("Input the elements:");
    for(int i=0; i != array_size; ++i){
        scanf("%d", &array1[i]);
    }

    //prepare the parameters for runner of thread
    para param[2];
    param[0].low = 0;
    param[0].high = array_size/2;
    param[1].low = array_size/2;
    param[1].high = array_size;

    //get the default attributes
    pthread_attr_init(&attr);

    //create the threads
    for(int i=0; i!=2; ++i){
        pthread_create(&tid[i], &attr, bubbleSort, &param[i]);
    }
    //wait for the threads exit
    for(int i=0; i!=2; ++i){
        pthread_join(tid[i], NULL);
    }

    //print the array sorted by two threads
    printf("Thread 0:\n");
    for(int i=0;i != array_size/2;++i){
        printf("%d ", array1[i]);
    }
    printf("\n");
    printf("Thread 1:\n");
    for(int i=array_size/2;i != array_size;++i){
        printf("%d ", array1[i]);
    }
    printf("\n");

    int middle = array_size/2;
    //create therad 2 to merge
    pthread_create(&tid[2], &attr, merge, &middle);
    //wait
    pthread_join(tid[2], NULL);

    //print sorted list
    printf("Sorted list:\n");
    for(int i = 0; i != array_size; ++i){
        printf("%d ", array2[i]);
    }
    printf("\n");

    return 0;

}

//Bubble Sort
void *bubbleSort(void *param){
    int low = ((para *) param)->low;
    int high = ((para *) param)->high;

    int i, j, tmp;
    int flag;
    for(i = 1; i<(high - low); ++i){
        flag = 0;
        for(j = low; j<high - i; ++j){
            if(array1[j+1] < array1[j]){
                tmp = array1[j];
                array1[j] = array1[j+1];
                array1[j+1] = tmp;
                flag = 1;
            }
        }
        if(!flag) break;
    }

    pthread_exit(0);
}

void *merge(void *param){
    int p1 = 0;
    int p2 = *((int *) param);
    int p3 = 0;

    while(p1 != array_size/2 || p2 != array_size){
        if(p1 == array_size/2){
            while (p2 != array_size)
            {
                array2[p3++] = array1[p2++];
            }
            break;
        }
        if(p2 == array_size){
            while(p1 != array_size/2){
                array2[p3++] = array1[p1++];
            }
            break;
        }
        if(array1[p1] < array1[p2]){
            array2[p3++] = array1[p1++];
        }else{
            array2[p3++] = array1[p2++];
        }
    }

    pthread_exit(0);
}