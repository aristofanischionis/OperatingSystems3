// in charge of the public ledger file

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include "myheader.h"

extern int errno;

int main(int argc, char *argv[])
{
    printf("hi im port master %d\n", argc);
    int shmid, req = 0;
    char chargesFile[20];
    int Scost = 0, Mcost = 0, Lcost = 0, cost = 0;
    char type[10];
    VesselInfo *ShipToEnter;
    FILE *fp;
    SharedMemory *myShared;
    if (argc != 5)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    //
    if (!strcmp(argv[1], "-c"))
    {
        strcpy(chargesFile, argv[2]);
    }
    else
    {
        printf("couldn't read the -c parameter");
        return 2;
    }
    fp = fopen(chargesFile, "r");
    while (fscanf(fp, "%s %d", type, &cost) == 2)
    {
        if (!strcmp(type, "Small"))
            Scost = cost;
        else if (!strcmp(type, "Medium"))
            Mcost = cost;
        else if (!strcmp(type, "Large"))
            Lcost = cost;
    }
    printf("costs: %d , %d , %d  \n", Scost, Mcost, Lcost);
    // port-master is in charge of portMovement sem and responsible to increment it when
    // a ship can move in the port
    // in charge of port movement sem
    printf("I am in");
    sem_wait(&(myShared->portMovement));
    sem_wait(&(myShared->OK));
    // takes first the port movement sem
    // has to decide if the first incoming ship can come in the port
    // curcap --; kai ++;
    while (1)
    {
        // waits for the first entry in shm vessel ship
        while (req != 0)
        {
            sleep(3);
            sem_getvalue(&(myShared->RequestEntry), &req);
            printf("port master req val is %d", req);
        }
        // now that there is some ship info in shm
        // let's check if I can place it somewhere
        if (myShared->shipToCome.type == 'S')
        {
            if (myShared->curcap1 > 0)
            {
                myShared->curcap1--;
                // all good I will give you the OK to move
                // and give you the OK to use the S sem
                myShared->shipToCome.stillINport = 1;
                // gave him permission to proceed
                // send the OK to read the status
                sem_post(&(myShared->OK));
                // you are free to move
                sem_post(&(myShared->portMovement));
                // going to park
                // write down the info I want for him
            }
            else if (myShared->curcap1 == 0)
            {
                // someone wants to exit port
                sem_wait(&(myShared->exit));
                // wait for port movement
                sem_wait(&(myShared->portMovement));
                myShared->curcap1++;
                // let the ones that are on the queue to go ahead
                // and do the vessel job
                sem_post(&(myShared->SmallSem));
            }
            else if (myShared->shipToCome.upgrade == 'M')
            {
                if (myShared->curcap2 > 0)
                {
                    myShared->curcap2--;
                    // all good I will give you the OK to move
                    // and give you the OK to use the m sem
                    myShared->shipToCome.stillINport = 1;
                    // gave him permission to proceed
                    // send the OK to read the status
                    sem_post(&(myShared->OK));
                    // you are free to move
                    sem_post(&(myShared->portMovement));
                }
                else if (myShared->curcap2 == 0)
                {
                    // haven't decided yet how to handle this case
                }
            }
            else if (myShared->shipToCome.upgrade == 'L')
            {
                if (myShared->curcap3 > 0)
                {
                    myShared->curcap3--;
                    // all good I will give you the OK to move
                    // and give you the OK to use the m sem
                    myShared->shipToCome.stillINport = 1;
                    // gave him permission to proceed
                    // send the OK to read the status
                    sem_post(&(myShared->OK));
                    // you are free to move
                    sem_post(&(myShared->portMovement));
                }
            }
        }
        else if (myShared->shipToCome.type == 'M')
        {
            // all good I will give you the OK to move
            // and give you the OK to use the M sem
            if (myShared->curcap2 > 0)
            {
                myShared->curcap2--;
                // all good I will give you the OK to move
                // and give you the OK to use the m sem
                myShared->shipToCome.stillINport = 1;
                // gave him permission to proceed
                // send the OK to read the status
                sem_post(&(myShared->OK));
                // you are free to move
                sem_post(&(myShared->portMovement));
                // going to park
                // write down the info I want for him
            }
            else if (myShared->curcap2 == 0)
            {
                // someone wants to exit port
                sem_wait(&(myShared->exit));
                // wait for port movement
                sem_wait(&(myShared->portMovement));
                myShared->curcap2++;
                // let the ones that are on the queue to go ahead
                // and do the vessel job
                sem_post(&(myShared->MedSem));
            }
            else if (myShared->shipToCome.upgrade == 'L')
            {
                if (myShared->curcap3 > 0)
                {
                    myShared->curcap3--;
                    // all good I will give you the OK to move
                    // and give you the OK to use the m sem
                    myShared->shipToCome.stillINport = 1;
                    // gave him permission to proceed
                    // send the OK to read the status
                    sem_post(&(myShared->OK));
                    // you are free to move
                    sem_post(&(myShared->portMovement));
                    // going to park
                    // write down the info I want for him
                }
            }
        }
        else if (myShared->shipToCome.type == 'L')
        {
            // all good I will give you the OK to move
            // and give you the OK to use the L sem
            if (myShared->curcap3 > 0)
            {
                myShared->curcap3--;
                // all good I will give you the OK to move
                // and give you the OK to use the l sem
                myShared->shipToCome.stillINport = 1;
                // gave him permission to proceed
                // send the OK to read the status
                sem_post(&(myShared->OK));
                // you are free to move
                sem_post(&(myShared->portMovement));
                // going to park
                // write down the info I want for him
            }
        }
    }

    exit(0);
    return 0;
}