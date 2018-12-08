#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include "vessel.h"

extern int errno;

struct configfile {
    char type1;
    char type2;
    char type3;
    int ca1;
    int ca2;
    int ca3;
    int co1;
    int co2;
    int co3;
};
typedef struct configfile configfile;

struct SharedMemory {
    //entrance and exit semaphores
    sem_t entranceSM;
    sem_t exitSM;
    // types
    configfile conf;
    // //
    char* publicLedger;
    char* logfile;
    VesselInfo ship;
};
typedef struct SharedMemory SharedMemory;


int main(int argc, char *argv[])
{
    int id=0, err=0;
    sem_t entranceSM, exitSM, *sp;
    char conFile[15], s1[10], s2[5];
    FILE *fp, *fp1, *fp2;
    char buff[15];
    // 
    // s1 = malloc(10 * sizeof(char));
    // s2 = malloc(5 * sizeof(char));
    // conFile = malloc(15 * sizeof(char));
    // buff = malloc(15 * sizeof(char));
    // make struct configfile
    configfile *struct_configfile = malloc(sizeof(configfile));
    if (struct_configfile == NULL) {printf("Couldn't malloc struct config file"); return 1; }
    //
    printf("BROMAAAAAAAAAAAAAAAAAAA");
    if(argc != 3){ printf("You haven't supplied enough arguments"); return 2; }
    if(!strcmp(argv[1], "-l")){
        strcpy(conFile, argv[2]);
    }
    else { printf("You haven't supplied -l parameter"); return 3; }
    // read from file
    fp = fopen(conFile, "r");
    
    while (fscanf(fp,"%s %s", s1, s2) == 2) {
        if(!strcmp(s1,"type1")){
            struct_configfile->type1 = s2[0];
            continue;
        }
        if(!strcmp(s1,"type2")){
            struct_configfile->type2 = s2[0];
            continue;
        }
        if(!strcmp(s1,"type3")){
            struct_configfile->type3 = s2[0];
            continue;
        }
        if(!strcmp(s1,"capacity1")){
            struct_configfile->ca1 = atoi(s2);
            continue;
        }
        if(!strcmp(s1,"capacity2")){
            struct_configfile->ca2 = atoi(s2);
            continue;
        }
        if(!strcmp(s1,"capacity3")){
            struct_configfile->ca3 = atoi(s2);
            continue;
        }
        if(!strcmp(s1,"cost1")){
            struct_configfile->co1 = atoi(s2);
            continue;
        }
        if(!strcmp(s1,"cost2")){
            struct_configfile->co2 = atoi(s2);
            continue;
        }
        if(!strcmp(s1,"cost3")){
            struct_configfile->co3 = atoi(s2);
            continue;
        }
        else {
            printf("I read something weird %s, %s \n", s1,s2);
            return 4;
        }
    }
    
    // make public ledger file
    fp1 = fopen("publicLedger", "a");
    // make log file
    fp2 = fopen("log", "a");
    // make shared memory
    id = shmget(IPC_PRIVATE ,sizeof(SharedMemory) ,0666); /*  Make  shared  memory  segment  */
    if (id ==  (void *) -1) {perror ("Creation"); exit(6);}
    else printf("Allocated. %d\n", (int)id);
    // attach to sp
    sp = (sem_t*)shmat(id ,(void*) 0, 0);
    if ( sp == (void *)  -1) { perror("Attachment."); exit (7);}

    /*  Initialize  the  semaphores. */
    if (sem_init(sp ,1,2) != 0) { perror("Couldn ’t initialize. sp"); exit(8); }
    if (sem_init(&entranceSM ,1,3) != 0) { perror("Couldn ’t initialize. entranceSM"); exit(9); }
    if (sem_init(&exitSM ,1,4) != 0) { perror("Couldn ’t initialize. exitSM"); exit(10); }
    // exec all programs
    // setsid sh -c 'exec command <> /dev/tty2 >&0 2>&1' to exec in a new tty
    // make sure you fork and exec your childern and then wait for them to finish



    // free malloc'ed space
    free(struct_configfile);
    // destroy sms
    sem_destroy(sp);
    sem_destroy(&entranceSM);
    sem_destroy(&exitSM);
    // delete shm seg
    err = shmctl(id, IPC_RMID , 0); /*  Remove  segment  */
    if(err ==  -1) perror("Removal.");
    else printf("Removed. %d\n", (int)(err));
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    // free strings
    // free(s1);
    // free(s2);
    // free(conFile);
    // free(buff);
    
    return  0;
}