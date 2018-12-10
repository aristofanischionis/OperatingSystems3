#include <semaphore.h>

struct Vessel
{
    char name[20];
    char type;
    char upgrade;
    int parkperiod;
    int mantime;
    float arrivalTime;
    float departureTime; 
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

struct PublicLedger {
    VesselInfo *CurVessels;
    // piasmenh h oxi
};
typedef struct PublicLedger PublicLedger;
// public ledger ena s ena m ena l

struct SharedMemory
{
    //entrance and exit semaphores, maybe just one for movement
    sem_t SmallSem;
    sem_t MedSem;
    sem_t LarSem;
    sem_t StoMsem;
    sem_t StoLsem;
    sem_t MtoLsem;
    //
    // int curcap1;
    // int curcap2;
    // int curcap3;
    // //
    char logfile[20];
    VesselInfo shipToCome;
    PublicLedger curState;
};
typedef struct SharedMemory SharedMemory;
