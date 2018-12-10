// in charge of the public ledger file

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include "myheader.h"

extern int errno;

int main(int argc, char *argv[]){
    printf("hi im port master %d\n", argc);
    int shmid;
    char chargesFile[20];
    int Scost = 0, Mcost = 0, Lcost = 0, cost = 0;
    char type[10];
    FILE *fp;
    SharedMemory *myShared;
    if(argc != 5){
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    //
    if (!strcmp(argv[1], "-c"))
    {
        strcpy(chargesFile, argv[2]);
    }
    else {
        printf("couldn't read the -c parameter");
        return 2;
    }
    fp = fopen(chargesFile, "r");
    while(fscanf(fp, "%s %d", type, &cost) == 2){
        printf(" type %s, cost %d \n", type, cost);
        if(!strcmp(type, "Small")) Scost = cost;
        else if(!strcmp(type, "Medium")) Mcost = cost;
        else if(!strcmp(type, "Large")) Lcost = cost;
    }
    printf("costs: %d , %d , %d  \n",Scost, Mcost, Lcost );




    exit(0);
    return 0;
}