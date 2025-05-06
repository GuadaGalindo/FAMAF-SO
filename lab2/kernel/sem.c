#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "sem.h"

 struct semaphore semArray[MAX_SEM]; 
//Implementacion de syscalls
//Abre y/o inicializa el semáforo “sem” con  un valor arbitrario “value”. 
    uint64 sys_sem_open (){
    int sem, value;

    argint (0,&sem);
    argint (1,&value);


    return sem_open (sem,value);
}

//Incrementa el semáforo ”sem” desbloqueando los procesos cuando su valor es 0. 
uint64 sys_sem_up (){
    int sem;

    argint (0,&sem);

    return sem_up(sem);

}

//decrementa el semáforo ”sem” bloqueando los procesos cuando su valor es 0. El valor del semaforo nunca puede ser menor a 0
uint64 sys_sem_down (){
    int sem;

    argint (0,&sem);

     return sem_down(sem);
}

//Libera el semáforo “sem”. 
uint64 sys_sem_close(){
    int sem;
        
    argint (0,&sem);

    return sem_close(sem);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Devuelven 1 en caso de 

int sem_open(int sem, int value){

    int found_sem = 0;

    if (sem < 0 || sem >= MAX_SEM || value < 0){ 
        printf("ERROR: problema de borde\n");  //error de borde
        return ERROR;                                   
    }

    while (sem < MAX_SEM && !found_sem)
    {
        if (semArray[sem].is_open == 0)
        {
            initlock (&semArray[sem].lock,"sem_new");

            acquire (&semArray[sem].lock);                      //bloqueamos asi otro proceso no abre el mismo sem    


            semArray[sem].value = value;
            semArray[sem].is_open = 1;

            release (&semArray[sem].lock);                      //ya inicializamos, liberamos el lock
            found_sem = 1;
        }
        else {
            sem++;
        }
        
    }
    return sem;
}


int sem_close(int sem){
    if (sem < 0 || sem >= MAX_SEM){ 
        printf("ERROR: problema de borde.\n");       //error de borde
        return ERROR;                                   
    }
    if (semArray[sem].is_open == 0){ 
        printf("ERROR: Semáforo ya cerrado.\n");       //semaforo ya cerrado
        return ERROR;                                  
    }
    acquire (&semArray[sem].lock);

    semArray[sem].is_open = 0;
    semArray[sem].value = 0;

    release (&semArray[sem].lock);

    return SUCCES;
}


int sem_up(int sem){

    if (sem < 0 || sem >= MAX_SEM){   
        printf("ERROR: problema de borde.\n");    //error de borde
        return ERROR;                                   
    }
    if (semArray[sem].is_open == 0){ 
        printf("ERROR: Semaforo cerrado\n");         //semaforo cerrado
        return ERROR;                                  
    }

    acquire (&semArray[sem].lock);
    
    semArray[sem].value = semArray[sem].value + 1u;

    if (semArray[sem].value == 1)
    {
        wakeup(&semArray[sem]);

    }
    release (&semArray[sem].lock);

    return SUCCES;
}

int sem_down(int sem){

    if (sem < 0 || sem >= MAX_SEM){  
        printf("ERROR: problema de borde.\n");       //error de borde
        return ERROR;                                   
    }
    if (semArray[sem].is_open == 0){  
        printf("ERROR: Semaforo cerrado\n");        //semaforo cerrado
        return ERROR;                                  
    }
    
    acquire (&semArray[sem].lock);

    
    if (semArray[sem].value > 0){

        semArray[sem].value --;
    }
    else{
        
        sleep(&semArray[sem], &semArray[sem].lock);
        semArray[sem].value = 0;
    }
    
    release (&semArray[sem].lock);

    return SUCCES;
}