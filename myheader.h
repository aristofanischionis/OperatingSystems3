#include <semaphore.h>

struct Vessel {

};

typedef struct Vessel VesselInfo;

struct configfile
{
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

struct SharedMemory
{
    //entrance and exit semaphores
    sem_t entranceSM;
    sem_t exitSM;
    // types
    configfile conf;
    // //
    char *publicLedger;
    char *logfile;
    VesselInfo ship;
};
typedef struct SharedMemory SharedMemory;
