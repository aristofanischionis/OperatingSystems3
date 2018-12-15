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
    // moving in the port in order to park
    printf("I am vessel and begin sleeping mantime, %s\n", myvessel->name);
    sleep(myvessel->mantime);
    // stopped moving so give permission to someone else to do work
    // printf("I am vessel and post req");
    sem_post(&(myShared->manDone));
    // printf("I am vessel and begin parkperiod");
    // stays in the port
    // printf("I am vessel and begin sleeping parkperiod, %s\n", myvessel->name);
    sleep((myvessel->parkperiod) / 2);
    // asks how much should I pay?

    sleep((myvessel->parkperiod) / 2);
    // I now want to exit
    sem_wait(&(myShared->Request));
    printf("I am vessel and sending exit status, %s\n", myvessel->name);
    myvessel->status = EXIT;
    memcpy(&(myShared->shipToCome), myvessel, sizeof(VesselInfo));
    // printf("vessel before ok wrote exit: %d",myShared->shipToCome.status );
    sem_post(&(myShared->OKves));
    // wait port-master to read from shm
    // sleep(2);
    //send OKpm i've sent my info to port master and wait for him to respond that he read it too
    sem_wait(&(myShared->OKpm));
    // printf("vessel after ok read: %d",myShared->shipToCome.status );
    // wait till you can leave port
    // time to move from port
    // printf("I am vessel and doing my last mantime, %s\n", myvessel->name);
    sleep(myvessel->mantime);
    //let the others know I 'm done using the port
    // printf("vessel is posting the mandone, %s\n", myvessel->name);
    sem_post(&(myShared->manDone));
    return;
}

int main(int argc, char *argv[])
{
    printf("this is a vessel %d\n", argc);
    int shmid;
    VesselInfo *myvessel;
    SharedMemory *myShared;
    if (argc != 11)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }    
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
        myvessel->upgrade = argv[4][0];
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
    myvessel->status = ENTER;
    myvessel->upgraded = NO;
    myvessel->cost = 0;
    strcpy(myvessel->pos, "N0");
    // attach shared mem
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(3);
    }
    SharedMemory *node = (SharedMemory*) myShared; 
    node->pubLedger.SmallVessels = (VesselInfo *)((uint8_t *)myShared + sizeof(SharedMemory));
    
    node->pubLedger.MediumVessels = (VesselInfo *)((uint8_t *)node->pubLedger.SmallVessels + \
    (node->curcap2)*sizeof(VesselInfo));

    node->pubLedger.LargeVessels = (VesselInfo *)((uint8_t *)node->pubLedger.MediumVessels + \
    (node->curcap3)*sizeof(VesselInfo));
    
    // begin doing stuff
    // ask for entry putting info in the shm
    // int req;
    // sem_getvalue(&(node->Request), &req);
    // printf("VSrequest is -> %d\n", req);
    // sem_getvalue(&(node->OKpm), &req);
    // printf("VSok is -> %d\n", req);
    sem_wait(&(node->Request));
    // putting my info for review
    // node->shipToCome = *myvessel;
    memcpy(&(node->shipToCome), myvessel, sizeof(VesselInfo));
    // send OKpm that I put info
    // printf("vessel before ok : %d",node->shipToCome.status );
    sem_post(&(node->OKves));
    // sleep(2);
    // wait for the OKpm from port master
    // printf("waiting ok from pm %s\n", node->shipToCome.name);
    sem_wait(&(node->OKpm));
    // printf("got ok %s %c\n", node->shipToCome.name, node->shipToCome.type);
    // printf("what a vessel read : %d",node->shipToCome.status );
    // let's check if I am eligible to park
    if (node->shipToCome.status == ACCEPTED)
    {
        printf("I am accepted %s\n", node->shipToCome.name);
        // YES I got accepted let's proceed
        vesselJob(myvessel, node);
    }
    else if (node->shipToCome.status == WAIT)
    {
        // didn't get accepted let's put my self in the correct fifo
        // depending on my type
        if (node->shipToCome.type == 'S')
        {
            // send a man done 
            // sem_post(&(myShared->manDone));
            // wait for the small semaphore
            // wait in the relevant fifo
            printf("SOMEONE IS REALLY WAITING RIGHT HERESMALL SEM %s\n", myvessel->name);
            node->pendSR++;
            sem_wait(&(node->SmallSem));
            printf("I went through the small sem %s\n", myvessel->name);
            vesselJob(myvessel, node);
        }
        else if (node->shipToCome.type == 'M')
        {
            // send a man done 
            // sem_post(&(myShared->manDone));
            // wait for the medium semaphore
            // wait in the relevant fifo
            printf("SOMEONE IS REALLY WAITING RIGHT med SEM %s\n", myvessel->name);
            node->pendMR++;
            sem_wait(&(node->MedSem));
            printf("I went through the med sem %s\n", myvessel->name);
            vesselJob(myvessel, node);
        }
        else if (node->shipToCome.type == 'L')
        {
            // send a man done 
            // sem_post(&(myShared->manDone));
            // wait for the large semaphore
            // wait in the relevant 
            printf("SOMEONE IS REALLY WAITING RIGHT large SEM %s\n", myvessel->name);
            node->pendLR++;
            sem_wait(&(node->LarSem));
            printf("I went through the large sem %s\n", myvessel->name);
            vesselJob(myvessel, node);
        }
    }
    // ask for movement in the port
    // ask for the semaphores according to its type and place info in the shm
    printf("vessel is now exiting, %s \n", myvessel->name);
    // free malloc'd space
    free(myvessel);
    int err ;
    err = shmdt (( void *) myShared );
    if ( err == -1 ) perror (" Detachment ");
    
    exit(0);
    return 0;
}