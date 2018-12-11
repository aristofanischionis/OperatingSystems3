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

void vesselJob(VesselInfo *myvessel, SharedMemory *myShared)
{
    // wait for being able to move in the port
    sem_wait(&(myShared->portMovement));
    printf("I went through the port movement sem %s\n", myvessel->name);
    // dostuff
    // moving in the port in order to park
    sleep(myvessel->mantime);
    // stopped moving
    sem_post(&(myShared->portMovement));
    // stays in the port
    sleep(myvessel->parkperiod);
    // asks how much should I pay?
    // I now want to exit
    sem_post(&(myShared->exit));
    // wait till you can leave port
    sem_wait(&(myShared->portMovement));
    // time to move from port
    sleep(myvessel->mantime);
    //let the others know I 'm done using the port
    sem_post(&(myShared->portMovement));
    return;
}

int main(int argc, char *argv[])
{
    printf("this is a vessel %d\n", argc);
    printf("HI THERE");
    int shmid;
    VesselInfo *myvessel;
    SharedMemory *myShared;
    if (argc != 11)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }    
    printf("HI THERE");
    //
    myvessel = malloc(sizeof(VesselInfo));
    //
    sprintf(myvessel->name, "vessel_%d", getpid());
    if (!strcmp(argv[1], "-t"))
    {
        myvessel->type = argv[2][0];
    }
    else
    {
        printf("couldn't read the -t parameter");
        return 2;
    }
    if (!strcmp(argv[3], "-u"))
    {
        myvessel->upgrade, argv[4][0];
    }
    else
    {
        printf("couldn't read the -u parameter");
        return 2;
    }
    if (!strcmp(argv[5], "-p"))
    {
        myvessel->parkperiod = atoi(argv[6]);
    }
    else
    {
        printf("couldn't read the -p parameter");
        return 2;
    }
    if (!strcmp(argv[7], "-m"))
    {
        myvessel->mantime = atoi(argv[8]);
    }
    else
    {
        printf("couldn't read the -m parameter");
        return 2;
    }
    if (!strcmp(argv[9], "-s"))
    {
        shmid = atoi(argv[10]);
    }
    else
    {
        printf("couldn't read the -s parameter");
        return 2;
    }
    myvessel->arrivalTime = 0.0;
    myvessel->departureTime = 0.0;
    myvessel->stillINport = 0;
    // attach shared mem
    printf("BEFORE ATTACHING SHARED");
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(3);
    }
    printf("AFTER ATTACHING SHARED");
    // begin doing stuff
    // ask for entry putting info in the shm
    sem_wait(&(myShared->RequestEntry));
    // putting my info for review
    memcpy(&(myShared->shipToCome), myvessel, sizeof(VesselInfo));
    // wait for the OK from port master
    printf("vessel before ok : %d",myShared->shipToCome.stillINport );
    sem_wait(&(myShared->OK));
    printf("what a vessel read : %d",myShared->shipToCome.stillINport );
    // let's check if I am eligible to park
    if (myShared->shipToCome.stillINport == 1)
    {
        // YES I got accepted let's proceed
        vesselJob(myvessel, myShared);
    }
    else
    {
        // didn't get accepted let's put my self in the correct fifo
        // depending on my type
        if (myvessel->type == 'S')
        {
            // wait for the small semaphore
            // wait in the relevant fifo
            sem_wait(&(myShared->SmallSem));
            printf("I went through the small sem %s\n", myvessel->name);
            vesselJob(myvessel, myShared);
        }
        if (myvessel->type == 'M')
        {
            // wait for the medium semaphore
            // wait in the relevant fifo
            sem_wait(&(myShared->MedSem));
            printf("I went through the med sem %s\n", myvessel->name);
            vesselJob(myvessel, myShared);
        }
        if (myvessel->type == 'L')
        {
            // wait for the large semaphore
            // wait in the relevant fifo
            sem_wait(&(myShared->LarSem));
            printf("I went through the large sem %s\n", myvessel->name);
            vesselJob(myvessel, myShared);
        }
    }
    // ask for movement in the port
    // ask for the semaphores according to its type and place info in the shm

    // sem_post(&(myShared->portMovement));
    // free malloc'd space
    free(myvessel);
    printf("vessel is now exiting \n");
    exit(0);
    return 0;
}