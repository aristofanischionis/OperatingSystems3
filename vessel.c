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
    printf("this is a vessel %d\n",argc);
    int shmid;
    VesselInfo *myvessel;
    SharedMemory *myShared;
    if(argc != 11){
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    char name[20];
    sprintf(name, "vessel_%d", getpid());
    //
    myvessel = malloc(sizeof(VesselInfo));
    //
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
    // begin doing stuff
    // ask for movement in the port
    sem_wait(&(myShared->portMovement));
    printf("I went through the port movement sem\n");
    // ask for the semaphores according to its type and place info in the shm
    if(myvessel->type == 'S'){
        printf("I went through the small sem\n");
        // wait for the small semaphore
        sem_wait(&(myShared->SmallSem));
        if(myvessel->upgrade == 'M'){
            printf("I went through the stom sem\n");
            sem_wait(&(myShared->StoMsem));
        }
        else if (myvessel->upgrade == 'L'){
            sem_wait(&(myShared->StoLsem));
        }
        // no upgrade given
    }
    if(myvessel->type == 'M'){
        // wait for the medium semaphore
        sem_wait(&(myShared->MedSem));
        if(myvessel->upgrade == 'L'){
            sem_wait(&(myShared->MtoLsem));
        }
        // no upgrade given
    }
    if(myvessel->type == 'L'){
        // wait for the large semaphore
        sem_wait(&(myShared->LarSem));
        
    }
    // free malloc'd space
    free(myvessel);

    exit(0);
    return 0;
}