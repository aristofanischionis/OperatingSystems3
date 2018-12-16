#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/time.h>
#include "myheader.h"

extern int errno;
struct timeval t0;

int calcCost(VesselInfo *myvessel, SharedMemory *myShared){
    // how much each type of ship will pay per hour
    struct timeval t3;
    int co1, co2, co3; 
    int curCost = 0;
    co1 = myShared->co1;
    co2 = myShared->co2;
    co3 = myShared->co3;
    //
    double time_in = 0.0, time_now;
    gettimeofday(&t3, NULL);
    time_now = (double)(t3.tv_usec - t0.tv_usec) / 1000000 + (double)(t3.tv_sec - t0.tv_sec);
    time_in = time_now - myvessel->arrivalTime;
    printf("I calced %f time and time now -> %f \n", time_in, time_now);
    if(myvessel->parktype == SMALL){
        return (int)((time_in * co1) / 30);
    }
    else if(myvessel->parktype == MED){
        return (int)((time_in * co2) / 30);
    }
    else if(myvessel->parktype == LARGE){
        return (int)((time_in * co3) / 30);
    }
    return -1;
}

void vesselJob(VesselInfo *myvessel, SharedMemory *myShared)
{
    struct timeval t2;
    int curCost = 0;
    // change the parktype
    myvessel->parktype = myShared->shipToCome.parktype;
    strcpy(myvessel->pos, myShared->shipToCome.pos);
    // int pos;
    // char type;
    // sscanf(myShared->shipToCome.pos, "%c%d", type, pos);
    // wait for being able to move in the port
    // moving in the port in order to park
    printf("I am vessel and begin sleeping mantime, %s\n", myvessel->name);
    sleep(myvessel->mantime);
    // stopped moving so give permission to someone else to do work
    sem_post(&(myShared->manDone));
    // stays in the port
    sleep((myvessel->parkperiod) / 2);
    // asks how much should I pay?
    myvessel->cost = calcCost(myvessel, myShared);
    printf("I am %s ves and I will pay %d till now\n", myvessel->name, myvessel->cost);
    sleep((myvessel->parkperiod) / 2);
    // I now want to exit
    sem_wait(&(myShared->Request));
    printf("I am vessel and sending exit status, %s\n", myvessel->name);
    gettimeofday(&t2, NULL);
    double time_spent = (double)(t2.tv_usec - t0.tv_usec) / 1000000 + (double)(t2.tv_sec - t0.tv_sec);
    printf("VESSEL departure TIME %s,  %f\n",myvessel->name , time_spent);
    // time from beginning of vessel till now is the departure time
    myvessel->departureTime = time_spent; 
    myvessel->status = EXIT;
    memcpy(&(myShared->shipToCome), myvessel, sizeof(VesselInfo));
    sem_post(&(myShared->OKves));
    // wait port-master to read from shm
    //send OKpm i've sent my info to port master and wait for him to respond that he read it too
    sem_wait(&(myShared->OKpm));
    // wait till you can leave port
    // time to move from port
    sleep(myvessel->mantime);
    //let the others know I 'm done using the port
    sem_post(&(myShared->manDone));
    return;
}

int main(int argc, char *argv[])
{
    printf("this is a vessel %d\n", argc);
    int shmid;
    struct timeval t1;
    VesselInfo *myvessel;
    SharedMemory *myShared;
    if (argc != 11)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }    
    //
    //place timer
    gettimeofday(&t0, NULL);
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
    myvessel->parktype = NO;
    myvessel->cost = 0;
    strcpy(myvessel->pos, "N0");
    // attach shared mem
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(3);
    }
    // myShared->pubLedger.SmallVessels = (CurrentState *)((uint8_t *)myShared + sizeof(SharedMemory));

    // myShared->pubLedger.MediumVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.SmallVessels + \
    // (myShared->max2)*sizeof(CurrentState));

    // myShared->pubLedger.LargeVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.MediumVessels + \
    // (myShared->max3)*sizeof(CurrentState));
    // begin doing stuff
    // ask for entry putting info in the shm
    
    sem_wait(&(myShared->Request));
    // putting my info for review
    memcpy(&(myShared->shipToCome), myvessel, sizeof(VesselInfo));
    // send OKpm that I put info
    sem_post(&(myShared->OKves));
    // wait for the OKpm from port master
    sem_wait(&(myShared->OKpm));
    // let's check if I am eligible to park
    if (myShared->shipToCome.status == ACCEPTED)
    {
        printf("I am accepted %s\n", myShared->shipToCome.name);
        gettimeofday(&t1, NULL);
        double time_spent = (double)(t1.tv_usec - t0.tv_usec) / 1000000 + (double)(t1.tv_sec - t0.tv_sec);
        printf("VESSEL ARRIVAL TIME %s,  %f\n",myvessel->name , time_spent);
        myvessel->arrivalTime = time_spent;
        // YES I got accepted let's proceed
        vesselJob(myvessel, myShared);
    }
    else if (myShared->shipToCome.status == WAIT)
    {
        // didn't get accepted let's put my self in the correct fifo
        // depending on my type
        if (myShared->shipToCome.type == 'S')
        {
            // wait for the small semaphore
            // wait in the relevant fifo
            printf("SOMEONE IS REALLY WAITING RIGHT HERESMALL SEM %s\n", myvessel->name);
            myShared->pendSR++;
            sem_wait(&(myShared->SmallSem));
            printf("I went through the small sem %s\n", myvessel->name);
            gettimeofday(&t1, NULL);
            double time_spent = (double)(t1.tv_usec - t0.tv_usec) / 1000000 + (double)(t1.tv_sec - t0.tv_sec);
            printf("VESSEL ARRIVAL TIME after WAIT %s,  %f\n",myvessel->name , time_spent);
            myvessel->arrivalTime = time_spent;
            vesselJob(myvessel, myShared);
        }
        else if (myShared->shipToCome.type == 'M')
        {
            // wait for the medium semaphore
            // wait in the relevant fifo
            printf("SOMEONE IS REALLY WAITING RIGHT med SEM %s\n", myvessel->name);
            myShared->pendMR++;
            sem_wait(&(myShared->MedSem));
            printf("I went through the med sem %s\n", myvessel->name);
            gettimeofday(&t1, NULL);
            double time_spent = (double)(t1.tv_usec - t0.tv_usec) / 1000000 + (double)(t1.tv_sec - t0.tv_sec);
            printf("VESSEL ARRIVAL TIME %s,  %f\n",myvessel->name , time_spent);
            myvessel->arrivalTime = time_spent;
            vesselJob(myvessel, myShared);
        }
        else if (myShared->shipToCome.type == 'L')
        {
            // wait for the large semaphore
            // wait in the relevant 
            printf("SOMEONE IS REALLY WAITING RIGHT large SEM %s\n", myvessel->name);
            myShared->pendLR++;
            sem_wait(&(myShared->LarSem));
            printf("I went through the large sem %s\n", myvessel->name);
            gettimeofday(&t1, NULL);
            double time_spent = (double)(t1.tv_usec - t0.tv_usec) / 1000000 + (double)(t1.tv_sec - t0.tv_sec);
            printf("VESSEL ARRIVAL TIME %s,  %f\n",myvessel->name , time_spent);
            myvessel->arrivalTime = time_spent;
            vesselJob(myvessel, myShared);
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