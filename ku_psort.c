#include "ku_psort.h"

/*
 * argv[1] : m - total number of integers to be sorted
 * argv[2] : n - number of processes
 * argv[3] : input file
 * argv[4] : output file
 */

int main(int argc, char * argv[]){
    int i = 0;
    int * sortArray;
    int inputsize = strtol(argv[1], NULL, 10);
    int numProcesses = strtol(argv[2], NULL, 10);


    if(argc < 5){
        perror("format error\n");
        exit(1);
    }else{
        sortArray = readInput(argv[3], inputsize);  //Read file
    }
   
    psort(sortArray, inputsize, numProcesses, argv[4]); //Sorting & Save

    return 0;    
}
