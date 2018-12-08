#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include "myheader.h"

extern int errno;

int main(int argc, char *argv[]){
    int shmid;
    VesselInfo *myvessel;
    SharedMemory *myShared;
    if(argc != 11){
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    char name[20];
    sprintf(name, "vessel_%d", getpid());
    strcpy(myvessel->name, name);
    if (!strcmp(argv[1], "-t"))
    {
        myvessel->type = argv[2][0];
    }
    else {
        printf("couldn't read the -t parameter");
        return 2;
    }
    if (!strcmp(argv[3], "-u"))
    {
        myvessel->upgrade, argv[4][0];
    }
    else {
        printf("couldn't read the -u parameter");
        return 2;
    }
    if (!strcmp(argv[5], "-p"))
    {
        myvessel->parkperiod = atoi(argv[6]);
    }
    else {
        printf("couldn't read the -p parameter");
        return 2;
    }
    if (!strcmp(argv[7], "-m"))
    {
        myvessel->mantime = atoi(argv[8]);
    }
    else {
        printf("couldn't read the -m parameter");
        return 2;
    }
    if (!strcmp(argv[9], "-s"))
    {
        shmid = atoi(argv[10]);
    }
    else {
        printf("couldn't read the -s parameter");
        return 2;
    }

    // attach shared mem
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(3);
    }
    printf("Finished successfully");
    return 0;
}