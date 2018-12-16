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
#define INIT 8

//
#define YES 9
#define NO 10

struct Vessel
{
    char name[20];
    char type;
    char upgrade;
    int parktype;
    int parkperiod;
    int mantime;
    double arrivalTime;
    double departureTime;
    int status;
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
    int parktype;
    int cost;
    double time_in;
    double time_out;
};
typedef struct history history;

struct CurrentState {
    char vesselname[20];
    char type;
    int occupied; // Yes No
    double time_in;
};
typedef struct CurrentState CurrentState;

struct PublicLedger {
    // current state of the port
    // arrays with positions as many as the capacity of the port ca1,ca2,ca3
    CurrentState *SmallVessels;
    CurrentState *MediumVessels;
    CurrentState *LargeVessels;
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
    int co1;
    int co2;
    int co3;
    //
    int totalIncome;
    char logfile[20];
    VesselInfo shipToCome;
    PublicLedger pubLedger;
};
typedef struct SharedMemory SharedMemory;
