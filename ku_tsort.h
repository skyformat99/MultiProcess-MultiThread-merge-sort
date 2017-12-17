#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define BUF_SIZE 100

//parameter to thread fucntion
struct Thrthr{
    int * array;
    int len;
    int position;
};

int * readInput(char * fileName, int inputNum);
int * quicksort(int * arr, int size);
int * tsort(int * inputs, int inputSize, int numThreads);
int * halfMergeSort(int * arr1, int * arr2, int size1, int size2);
void * merge_sort_thread(void * param);
void copyMemory(int* arr1, int* arr2, int size);
void writeOutput(char * fileFormat ,int * sortResult, int inputNum);

//thread fucntion -> quciksort for merge
void * merge_sort_thread(void * param){
    struct Thrthr * thr = param;
    int i = 0;
    int length = (thr->len);
    int pos = (thr->position);
    int * arr = (int*)malloc(sizeof(int)*length);

    arr = &((thr->array)[pos]);     //get inputs's address

    for(i = 0; i < length; i++){
        arr[i] = (thr->array)[pos+i];
    }

    arr = quicksort(arr, length);

    for(i = 0; i< length; i++){
        (thr->array)[pos+i] = arr[i];   //update inputs
    }

}

//copy array
void copyMemory(int * arr1, int * arr2, int size) {
    int i;
    for(i=0; i<size; i++) {
        arr1[i] = arr2[i];
    }
}

//main sort
int * tsort(int * inputs, int inputSize, int numThreads){
    int * result = (int*)malloc(sizeof(int)*inputSize);
    int start = 0;
    int i= 0;

    //divide inputs
    int baseFragmentSize = inputSize / numThreads; //base Size
    int numBiggerFragments = inputSize % numThreads;  //Num of base Size + 1
    int fragmentSize[numThreads]; //fragmentsize array as numThreads
    int fTotal[numThreads]; //sum of framentSize
    int numThreadsRemain;
    int loop;

    struct Thrthr thr[numThreads];  //parameter struct array to thread function

    // Allocate divided inputs to each threads
    for(i = 0; i < numBiggerFragments; i++) {   //Insert baseSize + 1
        fragmentSize[i] = baseFragmentSize + 1;
    }

    for(i = numBiggerFragments; i < numThreads; i++) {    //Insert baseSize
        fragmentSize[i] = baseFragmentSize;
    }

    //initalize struct value & array start positon
    for(i = 0; i < numThreads; i++){
        thr[i].len = fragmentSize[i];
        thr[i].array = inputs;
        thr[i].position = start;
        fTotal[i] = start;
        start += fragmentSize[i];
    }

    //number of threads
    pthread_t p_thread[numThreads];

    //create threads
    for(i = 0; i < numThreads; i++){
        pthread_create(&p_thread[i], NULL, merge_sort_thread, &thr[i]);
    }

    //join threads
    for(i = 0; i < numThreads; i++){
        pthread_join(p_thread[i], NULL);
    }

    //merge sort
    loop = 1;

    numThreadsRemain = numThreads;
    while(numThreadsRemain > 1){
        numThreadsRemain = (numThreadsRemain + 1) / 2;  //+1 --> case : odd number of threads

        for(i = 0; i < (numThreads + 1) / 2; i += loop ){   //recursive for merge count 
            int j;
            int resultSize = fragmentSize[2*i];

            if(2*i + loop >= numThreads) {
                copyMemory(result+fTotal[2*i], &inputs[fTotal[2*i]], resultSize); 
            } else{
                resultSize += fragmentSize[2*i + loop];
                
                //merge
                int * temp =  halfMergeSort(
                        &inputs[fTotal[2*i]],
                        &inputs[fTotal[2*i + loop]],
                        fragmentSize[2*i],
                        fragmentSize[2*i + loop]
                        );
             
                copyMemory(result+fTotal[2*i], temp, resultSize);
                fragmentSize[2*i] = resultSize;
            }

            /*
            // for debug --> check merge process
            for(j=0; j<inputSize; j++) {
                printf("%4d", result[j]);
            }
            printf("%6d", loop);
            printf("\n");
            */

        }
        loop *= 2;
        inputs = result;
    }

    return result;
}

//half-merge sort
int * halfMergeSort(int * arr1, int * arr2, int size1, int size2) {
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
    int * left = (int*) malloc(sizeof(int) * size);
    int * mid = (int*) malloc(sizeof(int) * size);
    int * right = (int*) malloc(sizeof(int) * size);

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
