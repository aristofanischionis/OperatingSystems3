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

void writePubLed(SharedMemory *myShared)
{
    int pos;
    char parkType, tobechecked;
    tobechecked = myShared->shipToCome.type;
    sscanf(myShared->shipToCome.pos, "%c%d", parkType, pos);
    // i will place it in the 
    // type array
    if( tobechecked == 'S'){
        memcpy(&(myShared->pubLedger.SmallVessels[pos]), &(myShared->shipToCome), sizeof(VesselInfo));
    }
    else if( tobechecked == 'M'){
        memcpy(&(myShared->pubLedger.MediumVessels[pos]), &(myShared->shipToCome), sizeof(VesselInfo));
    }
    else if( tobechecked == 'L'){
        memcpy(&(myShared->pubLedger.LargeVessels[pos]), &(myShared->shipToCome), sizeof(VesselInfo));
    }
}

void exiting(SharedMemory *myShared)
{
    // proceed to exit
    int res = 0, uped = -1;
    char tobechecked1, tobechecked2;
    tobechecked1 = myShared->shipToCome.type;
    tobechecked2 = myShared->shipToCome.upgrade;
    uped = myShared->shipToCome.upgraded;
    // VesselInfo *ShipToExit = malloc(sizeof(VesselInfo));
    printf("exiting portmaster \n");
    // sending ok
    sem_post(&(myShared->OKpm));
    // memcpy(ShipToExit, &(myShared->shipToCome), sizeof(VesselInfo));
    // wait for it to finish
    // printf("exiting waits for mandone from %s\n", myShared->shipToCome.name);
    sem_wait(&(myShared->manDone));
    // printf("exiting took mandone\n");
    //write status to left
    myShared->shipToCome.status = LEFT;
    // free a space for another ship to enter
    if(uped == YES){
        if(tobechecked2 == 'M'){
            // it means it has been upgraded to a med
            myShared->curcap2++;
            res = MED;
        }
        else if(tobechecked2 == 'L'){
            myShared->curcap3++;
            res = LARGE;
        }
    }
    else {
        if(tobechecked1 == 'S'){
            myShared->curcap1++;
            res = SMALL;
        }
        else if(tobechecked1 == 'M'){
            myShared->curcap2++;
            res = MED;
        }
        else if(tobechecked1 == 'L'){
            myShared->curcap3++;
            res = LARGE;
        }
    }
    // write it to public ledger
    // WARNING A PLACE WILL OPEN WHICH IS NOT THE LAST AND ON THIS POS 
    // THE ENTER WILL PLACE THE NEW VES
    writePubLed(myShared);
    // check what position is available so that I can place
    // someone from the waiting queue
    if (res == SMALL)
    {
        // check if someone needs the sem
        if (myShared->pendSR > 0)
        {
            // then someone needs the small sem
            // so I will give him permission to go ahead
            // printf("sm exited posting sem bc %d\n", myShared->pendSR);
            sem_post(&(myShared->SmallSem));
            sem_wait(&(myShared->manDone));
            myShared->pendSR--;
        }
    }
    else if (res == MED)
    {
        // check if someone needs the sem
        if (myShared->pendMR > 0)
        {
            // then someone needs the med sem
            // so I will give him permission to go ahead
            sem_post(&(myShared->MedSem));
            sem_wait(&(myShared->manDone));
            myShared->pendMR--;
        }
    }
    else if (res == LARGE)
    {
        // check if someone needs the sem
        if (myShared->pendLR > 0)
        {
            // then someone needs the large sem
            // so I will give him permission to go ahead
            sem_post(&(myShared->LarSem));
            sem_wait(&(myShared->manDone));
            myShared->pendLR--;
        }
    }
    writePubLed(myShared);
}

void smallPlace(SharedMemory *myShared)
{
    printf("There is a small place for me\n");
    myShared->curcap1--;
    // all good I will give you the OKpm to move
    // and give you the OKpm to use the S sem
    myShared->shipToCome.status = ACCEPTED;
    int posi = myShared->max1 - myShared->curcap1 - 1;
    sprintf(myShared->shipToCome.pos, "S%d", posi);
    printf("My position is %s\n", myShared->shipToCome.pos);
    // gave him permission to proceed
    // send the OKpm to read the status
    sem_post(&(myShared->OKpm));
    // you are free to move
    // wait for sleep to finish so that The request will be open for someone else
    sem_wait(&(myShared->manDone));
    // going to park
    // write down the info I want for him
}
void medPlace(SharedMemory *myShared, int up)
{
    printf("There is a med place for me\n");
    myShared->curcap2--;
    // all good I will give you the OKpm to move
    // and give you the OKpm to use the m sem
    myShared->shipToCome.status = ACCEPTED;
    int posi = myShared->max2 - myShared->curcap2 - 1;
    sprintf(myShared->shipToCome.pos, "M%d", posi);
    printf("My position is %s\n", myShared->shipToCome.pos);
    if (up == YES)
    {
        myShared->shipToCome.upgraded = YES;
    }
    // gave him permission to proceed
    // send the OKpm to read the status
    sem_post(&(myShared->OKpm));
    // you are free to move
    // wait for sleep to finish so that The request will be open for someone else
    sem_wait(&(myShared->manDone));
    // going to park
    // write down the info I want for him
}
void largePlace(SharedMemory *myShared, int up)
{
    printf("There is a large place for me\n");
    myShared->curcap3--;
    // all good I will give you the OKpm to move
    // and give you the OKpm to use the m sem
    myShared->shipToCome.status = ACCEPTED;
    int posi = myShared->max3 - myShared->curcap3 - 1;
    sprintf(myShared->shipToCome.pos, "L%d", posi);
    printf("My position is %s\n", myShared->shipToCome.pos);
    if (up == YES)
    {
        myShared->shipToCome.upgraded = YES;
    }
    // gave him permission to proceed
    // send the OKpm to read the status
    sem_post(&(myShared->OKpm));
    // you are free to move
    // wait for sleep to finish so that The request will be open for someone else
    sem_wait(&(myShared->manDone));
    // going to park
    // write down the info I want for him
}
void wait(SharedMemory *myShared)
{
    printf("blepw no upgrade SO wait \n");
    // wait signal
    myShared->shipToCome.status = WAIT;
    // send the ok from pm
    sem_post(&(myShared->OKpm));
}
void entry(SharedMemory *myShared)
{
    if (myShared->shipToCome.type == 'S')
    {
        if (myShared->curcap1 > 0)
        {
            smallPlace(myShared);
        }
        else if (myShared->curcap1 == 0)
        {
            printf("blepw capacity =0 gia small, %c \n", myShared->shipToCome.upgrade);
            if (myShared->shipToCome.upgrade == 'M')
            {
                if (myShared->curcap2 > 0)
                {
                    printf("blepw med upgrade gia small \n");
                    medPlace(myShared, YES);
                }
            }
            else if (myShared->shipToCome.upgrade == 'L')
            {
                if (myShared->curcap3 > 0)
                {
                    printf("blepw large upgrade gia small \n");
                    largePlace(myShared, YES);
                }
            }
            else
            {
                //no upgrade given
                wait(myShared);
            }
        }
    }
    else if (myShared->shipToCome.type == 'M')
    {
        if (myShared->curcap2 > 0)
        {
            medPlace(myShared, NO);
        }
        else if (myShared->curcap2 == 0)
        {
            if (myShared->shipToCome.upgrade == 'L')
            {
                if (myShared->curcap3 > 0)
                {
                    largePlace(myShared, YES);
                }
            }
            else
            {
                //no upgrade given
                wait(myShared);
            }
        }
    }
    else if (myShared->shipToCome.type == 'L')
    {
        if (myShared->curcap3 > 0)
        {
            largePlace(myShared, NO);
        }
        else if (myShared->curcap3 == 0)
        {
            wait(myShared);
        }
    }
    else {
        printf("Type of ship is not correct\n");
    }
    //
    writePubLed(myShared);
}

void handleRequest(SharedMemory *node)
{
    int res = 0, req = -1;
    sem_post(&(node->Request));
    // wait for someone to put info
    sem_wait(&(node->OKves));
    printf("Request status %d, %c, %s\n", node->shipToCome.status, node->shipToCome.type, node->shipToCome.name);
    if (node->shipToCome.status == EXIT)
    {
        // proceed to exit
        exiting(node);
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
    SharedMemory *node = (SharedMemory *)myShared;
    node->pubLedger.SmallVessels = (VesselInfo *)((uint8_t *)myShared + sizeof(SharedMemory));

    node->pubLedger.MediumVessels = (VesselInfo *)((uint8_t *)node->pubLedger.SmallVessels +
                                                   (node->curcap2) * sizeof(VesselInfo));

    node->pubLedger.LargeVessels = (VesselInfo *)((uint8_t *)node->pubLedger.MediumVessels +
                                                  (node->curcap3) * sizeof(VesselInfo));

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
        double time_spent = (double)(t1.tv_usec - t0.tv_usec) / 1000000 + (double)(t1.tv_sec - t0.tv_sec);
        printf("time spent on this request is %f\n", time_spent);
    }
    int err;
    err = shmdt((void *)myShared);
    if (err == -1)
        perror(" Detachment ");
    exit(0);
    return 0;
}