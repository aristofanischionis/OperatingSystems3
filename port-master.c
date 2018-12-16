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

void exiting(SharedMemory *myShared)
{
    // proceed to exit
    int res = 0, parked = 0, max = 0;
    parked = myShared->shipToCome.parktype;
    char nameofVes[20];
    strcpy(nameofVes, myShared->shipToCome.name);
    printf("exiting portmaster %d\n", parked);
    // sending ok
    sem_post(&(myShared->OKpm));
    // wait for it to finish
    sem_wait(&(myShared->manDone));
    //write status to left
    myShared->shipToCome.status = LEFT;
    // free a space for another ship to enter
    if (parked == SMALL)
    {
        myShared->curcap1++;
        res = SMALL;
        max = myShared->max1;
        // say that the pos is now not occupied
        //I have to find the name on the current state
        for (int i = 0; i < max; i++)
        {
            if (!strcmp(myShared->pubLedger.SmallVessels[i].vesselname, nameofVes))
            {
                // I found it
                printf("EXITING S, %s\n", myShared->pubLedger.SmallVessels[i].vesselname);
                myShared->pubLedger.SmallVessels[i].occupied = NO;
                break;
            }
        }
    }
    else if (parked == MED)
    {
        myShared->curcap2++;
        res = MED;
        max = myShared->max2;
        // say that the pos is now not occupied
        //I have to find the name on the current state
        for (int i = 0; i < max; i++)
        {
            if (!strcmp(myShared->pubLedger.MediumVessels[i].vesselname, nameofVes))
            {
                // I found it
                printf("EXITING M, %s\n", myShared->pubLedger.MediumVessels[i].vesselname);
                myShared->pubLedger.MediumVessels[i].occupied = NO;
                break;
            }
        }
    }
    else if (parked == LARGE)
    {
        myShared->curcap3++;
        res = LARGE;
        max = myShared->max3;
        // say that the pos is now not occupied
        //I have to find the name on the current state
        for (int i = 0; i < max; i++)
        {
            if (!strcmp(myShared->pubLedger.LargeVessels[i].vesselname, nameofVes))
            {
                // I found it
                printf("EXITING L, %s\n", myShared->pubLedger.LargeVessels[i].vesselname);
                myShared->pubLedger.LargeVessels[i].occupied = NO;
                break;
            }
        }
    }

    // write it to public ledger
    // check what position is available so that I can place
    // someone from the waiting queue
    if (res == SMALL)
    {
        // check if someone needs the sem
        if (myShared->pendSR > 0)
        {
            // then someone needs the small sem
            // so I will give him permission to go ahead
            printf("sm exited posting sem bc %d\n", myShared->pendSR);
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
    // writePubLed(myShared);
}

int findPosition(SharedMemory *myShared)
{
    int topark = myShared->shipToCome.parktype; // where is it going to actually park
    CurrentState *myvessels = malloc(sizeof(CurrentState));
    int max = 0;

    if (topark == SMALL)
    {
        max = myShared->max1;
        myvessels = myShared->pubLedger.SmallVessels;
    }
    else if (topark == MED)
    {
        max = myShared->max2;
        myvessels = myShared->pubLedger.MediumVessels;
    }
    else if (topark == LARGE)
    {
        max = myShared->max3;
        myvessels = myShared->pubLedger.LargeVessels;
    }
    else
    {
        printf("Something went wrong at findPosition %d\n", topark);
        return -1;
    }
    for (int i = 0; i < max; i++)
    {
        printf("prin to no %d\n", myvessels[i].occupied);
        if (myvessels[i].occupied == NO)
        {
            // I found the first free place to park that's great
            // let's write it down to public ledger
            myvessels[i].occupied = YES;
            myvessels[i].time_in = myShared->shipToCome.arrivalTime;
            myvessels[i].type = myShared->shipToCome.type;
            strcpy(myvessels[i].vesselname, myShared->shipToCome.name);
            return i;
        }
    }
    free(myvessels);
    return -2;
}

void smallPlace(SharedMemory *myShared)
{
    printf("There is a small place for me\n");
    myShared->curcap1--;
    // all good I will give you the OKpm to move
    // and give you the OKpm to use the S sem
    myShared->shipToCome.status = ACCEPTED;
    myShared->shipToCome.parktype = SMALL;
    int posi = findPosition(myShared);
    // int posi = myShared->max1 - myShared->curcap1 - 1;
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
    myShared->shipToCome.parktype = MED;
    // int posi = myShared->max2 - myShared->curcap2 - 1;
    int posi = findPosition(myShared);
    sprintf(myShared->shipToCome.pos, "M%d", posi);
    printf("My position is %s\n", myShared->shipToCome.pos);
    if (up == YES)
    {
        myShared->shipToCome.parktype = MED;
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
    myShared->shipToCome.parktype = LARGE;
    int posi = findPosition(myShared);
    // int posi = myShared->max3 - myShared->curcap3 - 1;
    sprintf(myShared->shipToCome.pos, "L%d", posi);
    printf("My position is %s\n", myShared->shipToCome.pos);
    if (up == YES)
    {
        myShared->shipToCome.parktype = LARGE;
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
    else
    {
        printf("Type of ship is not correct\n");
    }
    //
    // writePubLed(myShared);
}

void handleRequest(SharedMemory *myShared)
{
    int res = 0, req = -1;
    sem_post(&(myShared->Request));
    // wait for someone to put info
    sem_wait(&(myShared->OKves));
    printf("Request status %d, %c, %s\n", myShared->shipToCome.status, myShared->shipToCome.type, myShared->shipToCome.name);
    if (myShared->shipToCome.status == EXIT)
    {
        // proceed to exit
        exiting(myShared);
    }
    else if (myShared->shipToCome.status == ENTER)
    {
        // proceed to entry checks
        entry(myShared);
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
    myShared->pubLedger.SmallVessels = (CurrentState *)((uint8_t *)myShared + sizeof(SharedMemory));

    myShared->pubLedger.MediumVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.SmallVessels +
                                                         (myShared->max2) * sizeof(CurrentState));

    myShared->pubLedger.LargeVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.MediumVessels +
                                                        (myShared->max3) * sizeof(CurrentState));

    // a ship can move in the port
    // in charge of port movement sem
    // takes first the port movement sem
    // has to decide if the first incoming ship can come in the port
    while (1)
    {
        //place timer
        gettimeofday(&t0, NULL);
        handleRequest(myShared);
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