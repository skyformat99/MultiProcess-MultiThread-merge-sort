#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <mqueue.h>

#define MSG_SIZE 8
#define NAME_POSIX "/my_my"
#define BUF_SIZE 100

int * readInput(char * fileName, int inputNum);
void writeOutput(char * fileFormat ,int * sortResult, int inputNum);
int * quicksort(int * arr, int size);
void psort(int * inputs, int inputSize, int numProcesses, char * fileFormat);
void copyMemory(int* arr1, int* arr2, int size);
int * halfMergeSort(int * arr1, int * arr2, int size1, int size2);


//half-merge sort
int * halfMergeSort(in t* arr1, int * arr2, int size1, int size2) {
    int * result = (int*)malloc(sizeof(int) * (size1 + size2));
    int left = 0, right = 0;
    int i = 0;

    while(left < size1 && right < size2) {
        if(arr1[left] > arr2[right]) {
            result[left + right] = arr2[right++];
        }else {
            result[left + right] = arr1[left++];
        }
    }

    while(left < size1) {
        result[left + right] = arr1[left++];
    }

    while(right < size2) {
        result[left + right] = arr2[right++];
    }

    return result;
}


//copy array
void copyMemory(int * arr1, int * arr2, int size) {
    int i;

    for(i=0; i<size; i++) {
        arr1[i] = arr2[i];
    }
}

//Main sort
void psort(int * inputs, int inputSize, int numProcesses, char * fileFormat){
    int * result = (int*) malloc(sizeof(int) * inputSize);

    int pid[numProcesses];
    int parent_pid = getpid();

    int * fragment = NULL;
    int baseFragmentSize = inputSize / numProcesses; //base Size
    int numBiggerFragments = inputSize % numProcesses;  //Num of base Size + 1
    int fragmentSize[numProcesses]; //fragmentsize array as numProcesses

    struct mq_attr attr;
    unsigned int prio = 1;
    int processNo = 0, childId = 0;
    int i = 0, start = 0;


    //Message Queue
    mqd_t mqdes;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 8;
    mqdes = mq_open(NAME_POSIX, O_CREAT|O_RDWR, 0666, &attr);

    if((mqdes == (mqd_t)-1)){
        perror("Message open error!");
        exit(1);
    }


    // Allocate divided inputs to each process
    for(i = 0; i < numBiggerFragments; i++) {   //Insert baseSize + 1
        fragmentSize[i] = baseFragmentSize + 1;
    }

    for(i = numBiggerFragments; i < numProcesses; i++) {    //Insert baseSize
        fragmentSize[i] = baseFragmentSize;
    }


    //fork
    pid[0] = parent_pid;

    while(childId == 0 && processNo < numProcesses -1){
        childId = fork();

        if(childId == 0){
            pid[++processNo] = getpid();   //pid array --> process's pid
        } else if(childId < 0){
            perror("Fork Error\n");
            exit(0);
        }
    }

    //Divid inputs
    fragment = (int*)malloc(sizeof(int) * fragmentSize[processNo]);
    start = 0;

    for(i = 0; i < processNo; i++){
        start += fragmentSize[i];
    }

    for(i = 0; i < fragmentSize[processNo]; i++){
        fragment[i] = inputs[i + start];
    }

    //Quick sort for merge sort
    fragment = quicksort(fragment, fragmentSize[processNo]);

    //Message Que for Merge sort
    if(getpid() != pid[0]){     //child
        int s = 0;
        int value;
        int size = fragmentSize[processNo];
        for(s = 0; s < size; s++){
            value = fragment[s];

            if(mq_send(mqdes,(char*)&value, MSG_SIZE, prio) == -1){
                perror("send error\n");
                exit(1);
            }
        }
    }

    if(getpid() == pid[0]){     //First parents

        int i = 0;
        int value;
        int size = fragmentSize[0];
        int j;

        for(j = 0; j < size; j++){
            inputs[i] = fragment[j];
            i++;
        }


        for(j = 0; j < inputSize - fragmentSize[0]; j++){
            if(mq_receive(mqdes,(char*)&value, MSG_SIZE, &prio) == -1){
                perror("send error\n");
                exit(1);
            }else{
                inputs[i++] = value;
            }
        }

    }

 
    //Merge sort
    if(getpid() == pid[0]){

        int start[numProcesses]; //sum of framentSize
        int total = 0;
        for(i = 0; i < numProcesses; i++){
            start[i] = total;
            total += fragmentSize[i];
        }


        //merge algorithm
        int loop = 1;

        int numProcessesRemain = numProcesses;
        while(numProcessesRemain > 1){
            numProcessesRemain = (numProcessesRemain + 1) / 2;  //+1 --> case : odd number of processes

            for(i = 0; i < (numProcesses + 1) / 2; i += loop ){   //recursive for merge count 
                int j;
                int resultSize = fragmentSize[2*i];

                if(2*i + loop >= numProcesses) {
                    copyMemory(result+start[2*i], &inputs[start[2*i]], resultSize); 
                } else{
                    resultSize += fragmentSize[2*i + loop];

                    //merge
                    int* temp =  halfMergeSort(   
                            &inputs[start[2*i]],
                            &inputs[start[2*i + loop]],
                            fragmentSize[2*i],
                            fragmentSize[2*i + loop]
                            );

                    copyMemory(result+start[2*i], temp, resultSize);
                    fragmentSize[2*i] = resultSize;
                }

            }

            loop *= 2;
            inputs = result;
        }
        /*  //test code 
        for(i = 0; i < inputSize; i++){
            printf("result %d inputs %d\n", i,inputs[i]);
        */
        
        writeOutput(fileFormat,result,inputSize);   //save result as file

    }

    mq_close(mqdes);
    mq_unlink(NAME_POSIX);

    free(fragment);
}

//read Input.txt
int * readInput(char * fileName, int inputNum){
    int * result = (int*)malloc(sizeof(int)*inputNum);
    int count = 0;
    FILE * fp = fopen(fileName, "r");
    char buff[BUF_SIZE];

    if(fp != NULL){
        while(count < inputNum){
            fgets(buff, BUF_SIZE, fp);
            result[count++] = strtol(buff,NULL,10);
        }
    }
    else{
        printf("Can't read the given file : %s\n", fileName);
        exit(0);
    }

    return result;
}

//Save result as file
void writeOutput(char * fileFormat ,int * sortResult, int inputNum){
    FILE * fp = fopen(fileFormat, "w+");
    int count = 0;

    if(fp != NULL){
        while(count < inputNum)
            fprintf(fp,"%d\n",sortResult[count++]);
    }       
}   

//Quick sort 
int * quicksort(int * arr, int size) {
    int pivot = arr[0];
    int * result = (int*) malloc(sizeof(int) * size);
    int i = 0;
    int l = 0, m = 1, r = 0;
    int* left = (int*) malloc(sizeof(int) * size);
    int* mid = (int*) malloc(sizeof(int) * size);
    int* right = (int*) malloc(sizeof(int) * size);

    for(i = 1; i < size; i++) {
        if(arr[i] < pivot)      left[l++] = arr[i];
        else if(arr[i] > pivot) right[r++] = arr[i];
        else                    mid[m++] = arr[i];
    }   

    if(l > 1) left = quicksort(left, l);

    for(i = 0; i < l; i++) {
        result[i] = left[i];
    }   

    for(i = 0; i < m; i++) {
        result[i + l] = pivot;
    }

    if(r > 1) right = quicksort(right, r);

    for(i = 0; i < r; i++) {
        result[i + l + m] = right[i];
    }

    free(left);
    free(mid);
    free(right);

    return result;
}
