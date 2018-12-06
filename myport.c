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
    //
    char* publicLedger;
    char* logfile;
    VesselInfo ship;
};
typedef struct SharedMemory SharedMemory;


int main(int argc, char *argv[])
{
	int retval;
    int id=0, err=0;
    sem_t *entranceSM, *exitSM, *sp;
    char *conFile, *s1, *s2;
    FILE *fp;
    char buff[50];
    // make struct configfile
    configfile *struct_configfile = malloc(sizeof(configfile));
    if (struct_configfile == NULL) {printf("Couldn't malloc struct config file"); return 1; }
    //
    if(argc != 3){ printf("You haven't supplied enough arguments"); return 1; }
    if(!strcmp(argv[1], "-l")){
        strcpy(conFile, argv[2]);
    }
    else { printf("You haven't supplied -l parameter"); return 1; }
    // read from file
    fp = fopen(conFile, "r");
    while (fgets(buff, sizeof buff, fp) != NULL) {
        // process line here
        if (sscanf(buff, "%s %s", s1, s2) == 2) {
        // there weren 2 items to convert
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
                return 1;
            }
        }
        else {
            printf("I read something weird %s \n", s1);
            return 1;
        }
    }
    fclose(fp);
    // make public ledger file
    fp = fopen("/publicLedger", "a");
    fclose(fp);
    // make log file
    fp = fopen("/log", "a");
    fclose(fp);
    // make shared memory
    id = shmget(IPC_PRIVATE ,sizeof(SharedMemory) ,0666); /*  Make  shared  memory  segment  */
    if (id ==  (void *) -1) {perror ("Creation"); exit(2);}
    else printf("Allocated. %d\n", (int)id);
    // attach to sp
    sp = (sem_t  *)shmat(id ,(void*) 0, 0);
    if ( sp == (void *)  -1) { perror("Attachment."); exit (2);}

    /*  Initialize  the  semaphores. */
    retval = sem_init(sp ,1,2);
    if (retval != 0) { perror("Couldn ’t initialize. sp"); exit(3); }
    retval = sem_init(entranceSM ,1,2);
    if (retval != 0) { perror("Couldn ’t initialize. entranceSM"); exit(3); }
    retval = sem_init(exitSM ,1,2);
    if (retval != 0) { perror("Couldn ’t initialize. exitSM"); exit(3); }
    // exec all programs
    // setsid sh -c 'exec command <> /dev/tty2 >&0 2>&1' to exec in a new tty
    // make sure you fork and exec your childern and then wait for them to finish



    // free malloc'ed space
    free(struct_configfile);
    // destroy sms
    sem_destroy(sp);
    sem_destroy(entranceSM);
    sem_destroy(exitSM);
    // delete shm seg
    err = shmctl(id, IPC_RMID , 0); /*  Remove  segment  */
    if(err ==  -1) perror("Removal.");
    else printf("Removed. %d\n", (int)(err));
    
    return  0;
}