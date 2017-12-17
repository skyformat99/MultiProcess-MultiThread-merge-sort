#include "ku_tsort.h"

/*
 * argv[1] : m - total number of integers to be sorted
 * argv[2] : n - number of threads
 * argv[3] : input file
 * argv[4] : output file
 */


int main(int argc, char * argv[]){
    int i = 0;
    int * sortArray;
    int * sortResult;
    int inputsize = strtol(argv[1], NULL, 10);
    int numThreads = strtol(argv[2], NULL, 10);

    if(argc < 5){
        perror("format error\n");
        exit(1);
    }else{
        sortArray = readInput(argv[3], inputsize);
    }
    
    sortResult = tsort(sortArray, inputsize, numThreads);

    writeOutput(argv[4], sortResult, inputsize);

    free(sortArray);

    return 0;    
}
