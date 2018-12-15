// in charge of the public ledger file

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/time.h>
#include "myheader.h"

extern int errno;

int exiting(SharedMemory *myShared)
{
    // VesselInfo *ShipToExit = malloc(sizeof(VesselInfo));
    printf("exiting portmaster \n");
    // memcpy(ShipToExit, &(myShared->shipToCome), sizeof(VesselInfo));
    // wait for it to finish
    // printf("exiting waits for mandone from %s\n", myShared->shipToCome.name);
    sem_wait(&(myShared->manDone));
    // printf("exiting took mandone\n");
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
    if ((myShared->shipToCome.type == 'S') && (myShared->shipToCome.upgraded == NO))
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
            sem_wait(&(myShared->manDone));
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
                    myShared->shipToCome.upgraded = YES;
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // printf("I just gave the ok pm to s->m %s\n", myShared->shipToCome.name);
                    // you are free to move
                    // going to park
                    sem_wait(&(myShared->manDone));
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
                    myShared->shipToCome.upgraded = YES;
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // you are free to move
                    // going to park
                    sem_wait(&(myShared->manDone));
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
    if ((myShared->shipToCome.type == 'M') && (myShared->shipToCome.upgraded == NO))
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
            sem_wait(&(myShared->manDone));
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
                    myShared->shipToCome.upgraded = YES;
                    // send the OKpm to read the status
                    sem_post(&(myShared->OKpm));
                    // you are free to move
                    // going to park
                    sem_wait(&(myShared->manDone));
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
    if ((myShared->shipToCome.type == 'L') && (myShared->shipToCome.upgraded == NO))
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
            sem_wait(&(myShared->manDone));
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
    int res = 0, req = -1;
    sem_post(&(node->Request));
    // wait for someone to put info
    sem_wait(&(node->OKves));
    printf("Request status %d, %c, %s\n", node->shipToCome.status, node->shipToCome.type, node->shipToCome.name);
    if (node->shipToCome.status == EXIT)
    {
        sem_post(&(node->OKpm));
        // proceed to exit
        res = exiting(node);
        // check what position is available so that I can place 
        // someone from the waiting queue
        if(res == SMALL){
            // check if someone needs the sem
            if(node->pendSR > 0){
                // then someone needs the small sem
                // so I will give him permission to go ahead
                // printf("sm exited posting sem bc %d\n", node->pendSR);
                sem_post(&(node->SmallSem));
                sem_wait(&(node->manDone));
                node->pendSR--;
            }
        }
        else if(res == MED){
            // check if someone needs the sem
            if(node->pendMR > 0){
                // then someone needs the med sem
                // so I will give him permission to go ahead
                sem_post(&(node->MedSem));
                sem_wait(&(node->manDone));
                node->pendMR--;
            }
        }
        else if(res == LARGE){
            // check if someone needs the sem
            if(node->pendLR > 0){
                // then someone needs the large sem
                // so I will give him permission to go ahead
                sem_post(&(node->LarSem));
                sem_wait(&(node->manDone));
                node->pendLR--;
            }
        }
    }
    else if (node->shipToCome.status == ENTER)
    {
        // proceed to entry checks
        entry(node);
    }
}


int main(int argc, char *argv[])
{
    printf("hi im port master %d\n", argc);
    struct timeval t0, t1;
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
    node->pubLedger.SmallVessels = (VesselInfo *)((uint8_t *)myShared + sizeof(SharedMemory));
    
    node->pubLedger.MediumVessels = (VesselInfo *)((uint8_t *)node->pubLedger.SmallVessels + \
    (node->curcap2)*sizeof(VesselInfo));

    node->pubLedger.LargeVessels = (VesselInfo *)((uint8_t *)node->pubLedger.MediumVessels + \
    (node->curcap3)*sizeof(VesselInfo));
    
    // a ship can move in the port
    // in charge of port movement sem
    // takes first the port movement sem
    // has to decide if the first incoming ship can come in the port
    while (1)
    {
        //place timer
        gettimeofday(&t0, NULL);
        handleRequest(node);
        gettimeofday(&t1, NULL);
        double time_spent = (double) (t1.tv_usec - t0.tv_usec) / 1000000 + (double) (t1.tv_sec - t0.tv_sec);
        printf("time spent on this request is %f\n", time_spent);
        
    }
    int err;
    err = shmdt((void *)myShared);
    if (err == -1)
        perror(" Detachment ");
    exit(0);
    return 0;
}