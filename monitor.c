#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>

extern int errno;

int main(int argc, char *argv[]){
    printf("hi im monitor %d\n", argc);
    exit(0);
    return 0;
}