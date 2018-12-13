#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> 
#include "myheader.h"

pid_t pidPM;

void handle_sigint(int sig) 
{ 
    signal(SIGINT, handle_sigint);
    printf("Caught signal for port-master termination\n");
    kill(sig, pidPM);
}
extern int errno;

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);
    int shmid = 0, err = 0, vesnum = 0, curves = 0;
    pid_t pidVessel, pidMonitor;
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
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->ca1 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "capacity2"))
        {
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->ca2 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "capacity3"))
        {
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->ca3 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost1"))
        {
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->co1 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost2"))
        {
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->co2 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "cost3"))
        {
            sscanf(buff, "%s %s", s1, s2);
            struct_configfile->co3 = atoi(s2);
            continue;
        }
        if (!strcmp(s1, "./monitor"))
        {
            strcpy(monitorParams, buff);
            continue;
        }
        if (!strcmp(s1, "vesselnum"))
        {
            sscanf(buff, "%s %d", s1, &vesnum);
            vesselParam = malloc(vesnum * sizeof(char *));
            for(int i=0;i<vesnum;i++){
                vesselParam[i] = malloc(40 * sizeof(char));
            }
            continue;
        }
        if (!strcmp(s1, "./vessel"))
        {
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
    // find all the capacity
    int sumCa = struct_configfile->ca1 + struct_configfile->ca2 + struct_configfile->ca3 +1;
    // make log file
    fp1 = fopen("log", "a");
    // make history file
    fp2 = fopen("history", "a");
    // make shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory) + sumCa * sizeof(VesselInfo), IPC_CREAT|IPC_EXCL|0666); /*  Make  shared  memory  segment  */
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
    
    // inittialize myShared
    SharedMemory *node = (SharedMemory*) myShared;
    VesselInfo *nodeShip = malloc(sizeof(VesselInfo));
    strcpy(nodeShip->name , "noname");
    nodeShip->type = 'N';
    nodeShip->upgrade = 'N';
    nodeShip->parkperiod = 0;
    nodeShip->mantime = 0;
    nodeShip->arrivalTime = 0.0;
    nodeShip->departureTime = 0.0;
    nodeShip->status = 0;
    node->shipToCome = *nodeShip;
    //
    // PublicLedger *nodeledg = malloc(sizeof(PublicLedger) + sumCa*sizeof(VesselInfo));
    // node->pubLedger = *nodeledg;
    // node->pubLedger.SmallVessels = (VesselInfo *) myShared + sizeof(SharedMemory);
    // node->pubLedger.MediumVessels = (VesselInfo *) myShared + sizeof(SharedMemory);
    // node->pubLedger.LargeVessels = (VesselInfo *) myShared + sizeof(SharedMemory);
    node->pubLedger.SmallVessels = (char *)myShared + sizeof(SharedMemory);
    node->pubLedger.MediumVessels = (char *)myShared + sizeof(SharedMemory);
    node->pubLedger.LargeVessels = (char *)myShared + sizeof(SharedMemory);
    // node->pubLedger.MediumVessels = node->pubLedger.SmallVessels + struct_configfile->ca2;
    // node->pubLedger.LargeVessels = node->pubLedger.MediumVessels + struct_configfile->ca3;
    strcpy(node->pubLedger.history , "history.txt");
    for(int i=0;i<struct_configfile->ca1 ; i++){
        node->pubLedger.SmallVessels[i] = *nodeShip;
    }
    for(int i=0;i<struct_configfile->ca2 ; i++){
        node->pubLedger.MediumVessels[i] = *nodeShip;
    }
    for(int i=0;i<struct_configfile->ca3 ; i++){
        node->pubLedger.LargeVessels[i] = *nodeShip;
    }
    //
    node->curcap1 = struct_configfile->ca1;
    node->curcap2 = struct_configfile->ca2;
    node->curcap3 = struct_configfile->ca3;
    /*  Initialize  the  semaphores. */

    if (sem_init(&(node->SmallSem), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->MedSem), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->LarSem), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->Request), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->OKpm), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->OKves), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    if (sem_init(&(node->manDone), 1, 0) != 0)
    {
        perror("Couldn’t initialize.");
        exit(9);
    }
    //    
    //
    strcpy(node->logfile, "log");
    //
    // exec all programs
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
        params[4] = malloc(10 * sizeof(char));
        sprintf(params[4], "%d", shmid);
        params[5] = NULL;
        execvp("./port-master", params);
    }
    // else{
    //     wait(NULL);
    // }
    if ((pidMonitor = fork()) == -1)
    {
        perror(" fork ");
        exit(1);
    }
    if (pidMonitor == 0)
    {
        char *params[8];
        for(int j =0 ; j < 7;j++){
            params[j] = malloc(15 * sizeof(char));
        }
        sscanf(monitorParams , "%s %s %s %s %s %s", params[0], params[1], params[2], params[3], params[4], params[5]);
        //child
        sprintf(params[6], "%d", shmid);
        params[7] = NULL;
        execvp("./monitor", params);
    }
    // else {
    //     wait(NULL);
    // }
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
            char *params[12];
            for(int j =0 ; j < 11;j++){
                params[j] = malloc(15 * sizeof(char));
            }
            sscanf(vesselParam[i] , "%s %s %s %s %s %s %s %s %s %s", params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7], params[8], params[9]);
            sprintf(params[10], "%d", shmid);
            params[11] = NULL;
            execvp("./vessel", params);
        }
        else{
            // wait(NULL);
            continue;
        }
    }
    // parent waits for kid processes to finish
    for(int i=0;i< vesnum+2;i++){
        wait(NULL);
    }
    // while(1) sleep(1);
    printf("All Kids exited\n");
    // free malloc'ed space
    free(struct_configfile);
    for(int i=0;i<vesnum;i++){
        free(vesselParam[i]);
    }
    free(vesselParam);
    free(nodeShip);
    // free(nodeledg);
    // destroy sms
    sem_destroy(&(myShared->SmallSem));
    sem_destroy(&(myShared->MedSem));
    sem_destroy(&(myShared->LarSem));
    sem_destroy(&(myShared->Request));
    sem_destroy(&(myShared->OKpm));
    sem_destroy(&(myShared->OKves));
    sem_destroy(&(myShared->manDone));
    // delete shm seg
    err = shmctl(shmid, IPC_RMID, 0); /*  Remove  segment  */
    if (err == -1) perror("Removal.");
    else printf("Removed Shared Memory.\n");

    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    return 0;
}