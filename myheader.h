#include <semaphore.h>
// #define MAX 50
#define ENTER 0
#define EXIT 1
#define LEFT 2
#define WAIT 3
#define ACCEPTED 4
//
#define SMALL 5
#define MED 6
#define LARGE 7

struct Vessel
{
    char name[20];
    char type;
    char upgrade;
    int parkperiod;
    int mantime;
    float arrivalTime;
    float departureTime;
    int status;
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
    // current state of the port
    // arrays with positions as many as the capacity of the port ca1,ca2,ca3
    VesselInfo *SmallVessels;
    VesselInfo *MediumVessels;
    VesselInfo *LargeVessels;
    char history[20]; // name of history file to open for append
};
typedef struct PublicLedger PublicLedger;

struct SharedMemory
{
    //semaphores
    sem_t SmallSem;
    sem_t MedSem;
    sem_t LarSem;
    sem_t Request;
    sem_t OKpm;
    sem_t OKves;
    sem_t manDone;
    // sem_t StoMsem;
    // sem_t StoLsem;
    // sem_t MtoLsem;
    //
    int curcap1;
    int curcap2;
    int curcap3;
    // //
    char logfile[20];
    VesselInfo shipToCome;
    PublicLedger pubLedger;
};
typedef struct SharedMemory SharedMemory;
