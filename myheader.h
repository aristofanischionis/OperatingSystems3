#include <semaphore.h>
#include <stdint.h>
#include <inttypes.h>
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
//
#define YES 8
#define NO 9

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
    int upgraded;
    int cost;
    char pos[4];
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

struct history
{
    char vesselname[20];
    char parktype;
    int cost;
    float time_in;
    float time_out;
};
typedef struct history history;

struct PublicLedger {
    // current state of the port
    // arrays with positions as many as the capacity of the port ca1,ca2,ca3
    VesselInfo *SmallVessels;
    VesselInfo *MediumVessels;
    VesselInfo *LargeVessels;
    char historyFile[20]; // name of history file to open for append
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
    //
    int pendSR;
    int pendMR;
    int pendLR;
    //
    int curcap1;
    int curcap2;
    int curcap3;
    //
    int max1;
    int max2;
    int max3;
    // //
    char logfile[20];
    VesselInfo shipToCome;
    PublicLedger pubLedger;
};
typedef struct SharedMemory SharedMemory;
