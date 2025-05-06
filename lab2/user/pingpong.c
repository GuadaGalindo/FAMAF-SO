#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

#define ERROR 0
#define SUCCESS 1 
/* Procedimientos para modularizar*/

int main (int argc,char * argv[]){

//abrimos dos semaforos, uno para ping y otro para pong
    int sem_child = 1;
    int sem_father = 2;
    int sem_ping = sem_open(sem_child,1);              //semaforo ping inicializado en 1
    int sem_pong = sem_open(sem_father,0);              //semaforo pong incializado en 0

    int N = atoi(argv[1]);                      //cantidad de veces a imprimir

    
    if (N < 1 || argc < 2) {
        printf ("ERROR: in argument\n");
        return ERROR;
        }

    if (sem_ping == 0){
        printf ("ERROR: in open\n");
        return ERROR;
        }

    if (sem_pong == 0){
        printf ("ERROR: in open\n");
        return ERROR;
        }

    unsigned int pid = fork ();

    if (pid < 0){
        printf ("ERROR: in fork\n");
        return ERROR;
    }

    if (pid == 0){                              //estamos en el hijo
      for (uint i=0; i < N; ++i){
            int error = sem_down(sem_ping);
            if (error == 0){
                printf ("ERROR: in down\n");
                return ERROR;
            }
 
            printf ("ping \n");

            int error_2 = sem_up (sem_pong);
            if (error_2 == 0){
                printf ("ERROR: in up\n");
                return  ERROR;
                }
        }

    }
    else{                                       //estamos en el padre
        for (uint i=0; i < N; ++i){
            int error = sem_down(sem_pong);
             if (error == 0){
                printf ("ERROR: in down\n");
                return ERROR;
            }

            printf ("\t pong \n");

            int error_2 = sem_up(sem_ping);
            if (error_2 == 0){
                printf ("ERROR: in up\n");
                return ERROR;
            }
        }
        int error =sem_close(sem_ping);
        if (error == 0){
            printf ("ERROR: in close\n");
                return ERROR;
            }
        int error_two =sem_close(sem_pong);
        if (error_two == 0){
            printf ("ERROR: in close\n");
                return ERROR;
            }

    }
    return SUCCESS;
}