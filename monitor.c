#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <sys/time.h>
#include "myheader.h"

extern int errno;

void currentPort(SharedMemory *myShared)
{
    FILE *fp;
    int max1, max2, max3, i, fd;
    char origType[10];
    max1 = myShared->max1;
    max2 = myShared->max2;
    max3 = myShared->max3;
    fp = fopen("monitor.txt", "a");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening file logfile\n");
        exit(1);
    }
    fd = fileno(fp);
    flock(fd, LOCK_EX);
    // for small
    for (i = 0; i < max1; i++)
    {
        if (myShared->pubLedger.SmallVessels[i].occupied == YES)
        {
            fprintf(fp, "S%d\t%s\t%c\t%f\n", i, myShared->pubLedger.SmallVessels[i].vesselname, myShared->pubLedger.SmallVessels[i].type, myShared->pubLedger.SmallVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "S%d\tEMPTY\tEMPTY\tEMPTY\t", i);
            fflush(fp);
        }
    }
    //
    for (i = 0; i < max2; i++)
    {
        if (myShared->pubLedger.MediumVessels[i].occupied == YES)
        {
            fprintf(fp, "M%d\t%s\t%c\t%f\n", i, myShared->pubLedger.MediumVessels[i].vesselname, myShared->pubLedger.MediumVessels[i].type, myShared->pubLedger.MediumVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "M%d\tEMPTY\tEMPTY\tEMPTY\t", i);
            fflush(fp);
        }
    }
    //
    for (i = 0; i < max3; i++)
    {
        if (myShared->pubLedger.LargeVessels[i].occupied == YES)
        {
            fprintf(fp, "S%d\t%s\t%c\t%f\n", i, myShared->pubLedger.LargeVessels[i].vesselname, myShared->pubLedger.LargeVessels[i].type, myShared->pubLedger.LargeVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "S%d\tEMPTY\tEMPTY\tEMPTY\t", i);
            fflush(fp);
        }
    }
    fsync(fd); // syncrhonize with permanent storage (disk)
    flock(fd, LOCK_UN);
    fclose(fp);
}

void calcStatistics(SharedMemory *myShared)
{
    // waiting time
    // I will only check history file
    // char *
}

int main(int argc, char *argv[])
{
    printf("Hello its a monitor");
    FILE *fp;
    struct timeval t0, t1;
    SharedMemory *myShared;
    int t, stattimes, shmid;
    double time_spent;
    if (argc != 7)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    //
    //place timer
    gettimeofday(&t0, NULL);

    if (!strcmp(argv[1], "-d"))
    {
        t = atoi(argv[2]);
    }
    else
    {
        printf("couldn't read the -d parameter");
        return 2;
    }
    if (!strcmp(argv[3], "-t"))
    {
        stattimes = atoi(argv[4]);
    }
    else
    {
        printf("couldn't read the -t parameter");
        return 2;
    }
    if (!strcmp(argv[5], "-s"))
    {
        shmid = atoi(argv[6]);
    }
    else
    {
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

    fp = fopen("monitor.txt", "a");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening file logfile\n");
        exit(1);
    }
    fprintf(fp, "Position\tName\tOriginalType\tArrived\n");
    fclose(fp);
    //
    if (t > stattimes)
    {
        while (1)
        {
            printf("Eimai edwwww3333333333w");
            sleep(stattimes);
            calcStatistics(myShared);
            sleep(t - stattimes);
            currentPort(myShared);
        }
    }
    else if (t < stattimes)
    {
        while (1)
        {
            printf("Eimai edww22222222222www");
            sleep(t);
            currentPort(myShared);
            sleep(stattimes - t);
            calcStatistics(myShared);
        }
    }
    else
    {
        while (1)
        {
            printf("Eimai edwwww1111111w");
            sleep(t);
            currentPort(myShared);
            calcStatistics(myShared);
        }
    }
    //
    // exit(0);
    return 0;
}