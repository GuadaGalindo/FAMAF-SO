#ifndef SEM_H
#define SEM_H

#include "defs.h" 

#define SUCCES 1
#define ERROR 0
#define MAX_SEM 256

typedef struct semaphore {
    unsigned int value;                                 //valor del semaforo
    int is_open;                                        //indica si esta abierto el semaforo
    struct spinlock lock;                              //si esta bloqueado o no
} _sem;

extern struct semaphore semArray[MAX_SEM]; 

// Prototipos de las funciones definidas en sem.c
int sem_open(int sem, int value);
int sem_close(int sem);
int sem_up(int sem);
int sem_down(int sem);

#endif
