// in charge of the public ledger file

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include "myheader.h"

extern int errno;

int main(int argc, char *argv[]){
    printf("hi im port master %d\n", argc);
    exit(0);
    return 0;
}