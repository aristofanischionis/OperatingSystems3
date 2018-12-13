// in charge of the public ledger file

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include "myheader.h"

extern int errno;

int exiting(SharedMemory *myShared)
{
    // VesselInfo *ShipToExit = malloc(sizeof(VesselInfo));
    printf("exiting portmaster \n");
    // memcpy(ShipToExit, &(myShared->shipToCome), sizeof(VesselInfo));
    // wait for it to finish
    printf("exiting waits for mandone\n");
    sem_wait(&(myShared->manDone));
    printf("exiting took mandone\n");
    //write status to left
    myShared->shipToCome.status = LEFT;
    // free a space for another ship to enter
    if (myShared->shipToCome.type == 'S')
    {
        myShared->curcap1++;
        return SMALL;
    }
    else if (myShared->shipToCome.type == 'M')
    {
        myShared->curcap2++;
        return MED;
    }
    else if (myShared->shipToCome.type == 'L')
    {
        myShared->curcap3++;
        return LARGE;
    }
    // write it to public ledger
    // let someone else to make a request
    return 0;
}

void entry(SharedMemory *myShared)
{
    if (myShared->shipToCome.type == 'S')
    {
        if (myShared->curcap1 > 0)
        {
            printf("There is a small place for me\n");
            myShared->curcap1--;
            // all good I will give you the OKpm to move
            // and give you the OKpm to use the S sem
            myShared->shipToCome.status = ACCEPTED;
            // gave him permission to proceed
            // send the OKpm to read the status
            sem_post(&(myShared->OKpm));
            // you are free to move
            // wait for sleep to finish so that The request will be open for someone else
            // going to park
            // write down the info I want for him
        }
        else if (myShared->curcap1 == 0)
        {
            printf("blepw capacity =0 gia small, %c \n", myShared->shipToCome.upgrade);
            if(myShared->shipToCome.upgrade == 'M'){
                if (myShared->curcap2 > 0)
                {
                    printf("blepw med upgrade gia small \n");
                    myShared->curcap2--;
                    // all good I will give you the OKpm to move
                    // and give you the OKpm to use the m sem
                    myShared->shipToCome.status = ACCEPTED;
                    // gave him permission to proceed
                    // change type to M
                    myShared->shipToCome.type = 'M';
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // you are free to move
                    // going to park
                    // write down the info I want for him
                }
            }
            else if(myShared->shipToCome.upgrade == 'L'){
                if (myShared->curcap3 > 0)
                {
                    myShared->curcap3--;
                    // all good I will give you the OKpm to move
                    // and give you the OKpm to use the m sem
                    myShared->shipToCome.status = ACCEPTED;
                    // gave him permission to proceed
                    // change type to M
                    myShared->shipToCome.type = 'L';
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // you are free to move
                    // going to park
                    // write down the info I want for him
                }
            }
            else {
                //no upgrade given
                printf("blepw no upgrade gia small \n");
                // wait signal
                myShared->shipToCome.status = WAIT;
                // send the ok from pm
                sem_post(&(myShared->OKpm));
            }
        }
        // means that this ship is already taken care of
        // myShared->shipToCome.type = 'N';
    }
    if (myShared->shipToCome.type == 'M')
    {
        if (myShared->curcap2 > 0)
        {
            printf("There is a med place for me\n");
            myShared->curcap2--;
            // all good I will give you the OKpm to move
            // and give you the OKpm to use the m sem
            myShared->shipToCome.status = ACCEPTED;
            // gave him permission to proceed
            // send the OKpm to read the status
            sem_post(&(myShared->OKpm));
            // you are free to move
            // wait for sleep to finish so that The request will be open for someone else
            // sem_wait(&(myShared->Request));
            // going to park
            // write down the info I want for him
        }
        else if (myShared->curcap2 == 0)
        {
            if(myShared->shipToCome.upgrade == 'L'){
                if (myShared->curcap3 > 0)
                {
                    myShared->curcap3--;
                    // all good I will give you the OKpm to move
                    // and give you the OKpm to use the m sem
                    myShared->shipToCome.status = ACCEPTED;
                    // gave him permission to proceed
                    // change type to M
                    myShared->shipToCome.type = 'L';
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // you are free to move
                    // going to park
                    // write down the info I want for him
                }
            }
            else {
                //no upgrade given
                // wait signal
                myShared->shipToCome.status = WAIT;
                sem_post(&(myShared->OKpm));
            }
        }
        // means that this ship is already taken care of
        // myShared->shipToCome.type = 'N';
    }
    if (myShared->shipToCome.type == 'L')
    {
        if (myShared->curcap3 > 0)
        {
            printf("There is a large place for me\n");
            myShared->curcap3--;
            // all good I will give you the OKpm to move
            // and give you the OKpm to use the m sem
            myShared->shipToCome.status = ACCEPTED;
            // gave him permission to proceed
            // send the OKpm to read the status
            sem_post(&(myShared->OKpm));
            // you are free to move
            // wait for sleep to finish so that The request will be open for someone else
            // sem_wait(&(myShared->Request));
            // going to park
            // write down the info I want for him
        }
        else if (myShared->curcap3 == 0)
        {
            // wait signal
            myShared->shipToCome.status = WAIT;
            sem_post(&(myShared->OKpm));
        }
        // means that this ship is already taken care of
        // myShared->shipToCome.type = 'N';
    }
}

void handleRequest(SharedMemory *node){
    int res = 0;
    sem_post(&(node->Request));
    // wait for someone to put info
    sem_wait(&(node->OKves));
    printf("Request status %d, %c\n", node->shipToCome.status, node->shipToCome.type);
    if (node->shipToCome.status == EXIT)
    {
        sem_post(&(node->OKpm));
        // proceed to exit
        res = exiting(node);
        // check what position is available so that I can place 
        // someone from the waiting queue
        if(res == SMALL){
            // post the sem
            sem_post(&(node->SmallSem));
            // wait for it to finish man
            // sem_wait(&(node->manDone));
        }
        else if(res == MED){
            // post the sem
            sem_post(&(node->MedSem));
            // wait for it to finish man
            // sem_wait(&(node->manDone));
        }
        else if(res == LARGE){
            // post the sem
            sem_post(&(node->LarSem));
            // wait for it to finish man
            // sem_wait(&(node->manDone));
        }
    }
    else if (node->shipToCome.status == ENTER)
    {
        // proceed to entry checks
        entry(node);
        // wait for vessel to post request so that the next request can be processed
        // sem_wait(&(node->manDone));
    }
}


int main(int argc, char *argv[])
{
    printf("hi im port master %d\n", argc);
    int shmid, req = 0;
    char chargesFile[20];
    int Scost = 0, Mcost = 0, Lcost = 0, cost = 0;
    char type[10];
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
    if (!strcmp(argv[3], "-s"))
    {
        shmid = atoi(argv[4]);
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
    // attach shm
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(3);
    }
    SharedMemory *node = (SharedMemory*) myShared; 
    node->pubLedger.SmallVessels = (char*)myShared + sizeof(SharedMemory);
    node->pubLedger.MediumVessels = (char*)myShared + sizeof(SharedMemory);
    node->pubLedger.LargeVessels = (char*)myShared + sizeof(SharedMemory);
    // a ship can move in the port
    // in charge of port movement sem
    // takes first the port movement sem
    // has to decide if the first incoming ship can come in the port
    while (1)
    {
        handleRequest(node);
        // sem_post(&(node->Request));
        // // wait for someone to put info
        // sem_wait(&(node->OKves));
        // printf("Request status %d, %c\n", node->shipToCome.status, node->shipToCome.type);
        
        // if (node->shipToCome.status == EXIT)
        // {
        //     // proceed to exit
        //     exiting(node);
        //     // sem_wait(&(node->manDone)); // if a vessel has come because of sem post
        // }
        // else if (node->shipToCome.status == ENTER)
        // {
        //     // proceed to entry checks
        //     entry(node);
        //     // wait for vessel to post request so that the next request can be processed
        //     sem_wait(&(node->manDone));
        // }
        // // now that there is some ship info in shm
        // // let's check if I can place it somewhere
    }
    int err;
    err = shmdt((void *)myShared);
    if (err == -1)
        perror(" Detachment ");
    exit(0);
    return 0;
}