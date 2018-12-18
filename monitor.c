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
            fprintf(fp, "S%d\t%s\t%c\t%.2f\n", i, myShared->pubLedger.SmallVessels[i].vesselname, myShared->pubLedger.SmallVessels[i].type, myShared->pubLedger.SmallVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "S%d\tEMPTY\tEMPTY\tEMPTY\t\n", i);
            fflush(fp);
        }
    }
    //
    for (i = 0; i < max2; i++)
    {
        if (myShared->pubLedger.MediumVessels[i].occupied == YES)
        {
            fprintf(fp, "M%d\t%s\t%c\t%.2f\n", i, myShared->pubLedger.MediumVessels[i].vesselname, myShared->pubLedger.MediumVessels[i].type, myShared->pubLedger.MediumVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "M%d\tEMPTY\tEMPTY\tEMPTY\t\n", i);
            fflush(fp);
        }
    }
    //
    for (i = 0; i < max3; i++)
    {
        if (myShared->pubLedger.LargeVessels[i].occupied == YES)
        {
            fprintf(fp, "L%d\t%s\t%c\t%.2f\n", i, myShared->pubLedger.LargeVessels[i].vesselname, myShared->pubLedger.LargeVessels[i].type, myShared->pubLedger.LargeVessels[i].time_in);
            fflush(fp);
        }
        else
        {
            fprintf(fp, "L%d\tEMPTY\tEMPTY\tEMPTY\t\n", i);
            fflush(fp);
        }
    }
    fsync(fd); // syncrhonize with permanent storage (disk)
    flock(fd, LOCK_UN);
    fclose(fp);
}

double average(double array[], int num){
    double sum= 0.0;
    for(int i=0 ; i < num;i++){
        sum += array[i];
    }
    return sum/num;
}

void calcStatistics(SharedMemory *myShared)
{
    // waiting time
    // I will only check history file
    // char *
    char buff[100];
    int fd, lines = 0, i=0, num = 0;
    char ch;
    FILE *fp;
    fp = fopen(myShared->pubLedger.historyFile, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening file logfile\n");
        exit(1);
    }
    fd = fileno(fp);
    flock(fd, LOCK_EX);
    // ignoring the first two lines
    while(!feof(fp))
    {
        ch = fgetc(fp);
        if(ch == '\n')
        {
            lines++;
        }
    }
    printf("lines in history %d\n", lines);
    fflush(stdout);
    if(lines <= 2){
        return ;
    }
    history his[lines];
    fscanf(fp,"%[^\n]", buff);
    fscanf(fp,"%[^\n]", buff);
    // let's read actual data
    while (fscanf(fp, "%s\t%s\t%lf\t%lf\t%lf\t%lf\t%d\n", his[i].vesselname, his[i].parktype, &his[i].reqEntry, &his[i].time_in, &his[i].reqExit, &his[i].time_out, &his[i].cost) > 0)
    {
        printf("ves name is %s\n", his[i].vesselname);
        i++;
    }
    num = i-1;
    double ArReqEntry[num], ArReqExit[num], ArCost[num];
    for(int j =0;j<num;j++){
        ArReqEntry[j] = his[j].reqEntry;
        ArReqExit[j] = his[j].reqExit;
        ArCost[j] = (double)his[j].cost;
    }
    double averReqEntry = average(ArReqEntry, num);
    double averReqExit = average(ArReqExit, num);
    double averInco = average(ArCost, num);
    // still need to calculate wait per type
    printf("average wait in entry %.2f\n", averReqEntry);
    printf("average wait in exit %.2f\n", averReqExit);
    printf("average income per vessel is %d\n", (int)averInco);
    printf("Total Income till now is %d\n", myShared->totalIncome);
    fflush(stdout);
    fsync(fd); // syncrhonize with permanent storage (disk)
    flock(fd, LOCK_UN);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    SharedMemory *myShared;
    int t, stattimes, shmid;
    if (argc != 7)
    {
        printf("not provided the right number of params %d", argc);
        return 1;
    }
    //

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
    // myShared->pubLedger.SmallVessels = (CurrentState *)((uint8_t *)myShared + sizeof(SharedMemory));

    // myShared->pubLedger.MediumVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.SmallVessels + \
    // (myShared->max2)*sizeof(CurrentState));

    // myShared->pubLedger.LargeVessels = (CurrentState *)((uint8_t *)myShared->pubLedger.MediumVessels + \
    // (myShared->max3)*sizeof(CurrentState));

    // begin doing stuff

    fp = fopen("monitor.txt", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening file logfile\n");
        exit(1);
    }
    fprintf(fp, "Position\tName\tOriginalType\tArrived\n");
    fflush(fp);
    fclose(fp);
    //

    if (t > stattimes)
    {
        while (1)
        {
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
            sleep(t);
            currentPort(myShared);
            calcStatistics(myShared);
        }
    }
    //
    // exit(0);
    return 0;
}