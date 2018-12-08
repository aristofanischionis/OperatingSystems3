#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include "myheader.h"

extern int errno;

int main(int argc, char *argv[])
{
    int shmid = 0, err = 0, vesnum = 0, curves = 0;
    pid_t pidPM, pidVessel, pidMonitor;
    sem_t entranceSM, exitSM;
    char conFile[15], buff[50], s1[10], s2[5];
    char monitorParams[30];
    FILE *fp, *fp1, *fp2;
    char **vesselParam;
    SharedMemory *myShared;
    //
    // make struct configfile
    configfile *struct_configfile = malloc(sizeof(configfile));
    if (struct_configfile == NULL)
    {
        printf("Couldn't malloc struct config file");
        return 1;
    }
    //
    if (argc != 3)
    {
        printf("You haven't supplied enough arguments");
        return 2;
    }
    if (!strcmp(argv[1], "-l"))
    {
        strcpy(conFile, argv[2]);
    }
    else
    {
        printf("You haven't supplied -l parameter");
        return 3;
    }
    // read from file
    fp = fopen(conFile, "r");

    while (fgets(buff, 50, fp) != NULL)
    {
        sscanf(buff, "%s", s1);
        if (!strcmp(s1, "type1"))
        {
            struct_configfile->type1 = s2[0];
            continue;
        }
        if (!strcmp(s1, "type2"))
        {
            struct_configfile->type2 = s2[0];
            continue;
        }
        if (!strcmp(s1, "type3"))
        {
            struct_configfile->type3 = s2[0];
            continue;
        }
        if (!strcmp(s1, "capacity1"))
        {
            struct_configfile->ca1 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "capacity2"))
        {
            struct_configfile->ca2 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "capacity3"))
        {
            struct_configfile->ca3 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost1"))
        {
            struct_configfile->co1 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost2"))
        {
            struct_configfile->co2 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost3"))
        {
            struct_configfile->co3 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "./monitor"))
        {
            printf("the buff in monitor is %s", buff);
            strcpy(monitorParams, buff);
            continue;
        }
        if (!strcmp(s1, "vesselnum"))
        {
            sscanf(buff, "%s %d", s1, &vesnum);
            vesselParam = malloc(vesnum * sizeof(char *));
            continue;
        }
        if (!strcmp(s1, "./vessel"))
        {
            printf("the buff in vessel is %s", buff);
            strcpy(vesselParam[curves], buff);
            curves++;
            continue;
        }
        else
        {
            printf("I read something weird %s, %s \n", s1, s2);
            return 4;
        }
    }

    fclose(fp);

    // make public ledger file
    fp1 = fopen("publicLedger", "a");
    // make log file
    fp2 = fopen("log", "a");
    // make shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666); /*  Make  shared  memory  segment  */
    if (shmid == (void *)-1)
    {
        perror("Creation");
        exit(6);
    }
    else
        printf("Allocated. %d\n", (int)shmid);
    // attach to myShared
    myShared = (SharedMemory *)shmat(shmid, (void *)0, 0);
    if (myShared == (void *)-1)
    {
        perror("Attachment.");
        exit(7);
    }

    /*  Initialize  the  semaphores. */

    if (sem_init(&entranceSM, 1, 3) != 0)
    {
        perror("Couldn ’t initialize. entranceSM");
        exit(9);
    }
    if (sem_init(&exitSM, 1, 4) != 0)
    {
        perror("Couldn ’t initialize. exitSM");
        exit(10);
    }
    // inittialize myShared

    // exec all programs
    // setsid sh -c 'exec command <> /dev/tty2 >&0 2>&1' to exec in a new tty
    // make sure you fork and exec your childern and then wait for them to finish
    if ((pidPM = fork()) == -1)
    {
        perror(" fork ");
        exit(1);
    }
    if (pidPM == 0)
    {
        //child
        char *params[6];
        params[0] = "./port-master";
        params[1] = "-c";
        params[2] = "charges";
        params[3] = "-s";
        sprintf(params[4], "%d", shmid);
        params[5] = NULL;
        execvp("./port-master", params);
    }
    if ((pidMonitor = fork()) == -1)
    {
        perror(" fork ");
        exit(1);
    }
    if (pidMonitor == 0)
    {
        //child
        char *params[3];
        strcpy(params[0], monitorParams);
        sprintf(params[1], "%d", shmid);
        params[2] = NULL;
        execvp("./monitor", params);
    }
    // vessels will get values from the configfile
    for (int i = 0; i < vesnum; i++)
    {
        if ((pidVessel = fork()) == -1)
        {
            perror(" fork ");
            exit(1);
        }
        if (pidVessel == 0)
        {
            //child
            char *params[3];
            strcpy(params[0], vesselParam[i]);
            sprintf(params[1], "%d", shmid);
            params[2] = NULL;
            execvp("./vessel", params);
        }
    }
    // parent waits for kid processes to finish
    // waits for kids to finish with a semaphore

    // free malloc'ed space
    free(struct_configfile);
    // destroy sms
    sem_destroy(&entranceSM);
    sem_destroy(&exitSM);
    // delete shm seg
    err = shmctl(shmid, IPC_RMID, 0); /*  Remove  segment  */
    if (err == -1)
        perror("Removal.");
    else
        printf("Removed. %d\n", (int)(err));

    fclose(fp1);
    fclose(fp2);
    return 0;
}