#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "tests/syscall_mock.h"
#include <sys/stat.h>

#include "execute.h"
#include "builtin.h"

//corroborar foreground
//corroborar background
//redirigir entrada antes de usar exec
//armar pipeline para conectar comandos

//PRIMERA TAREA: reconocer comandos internos y externos
//decidir si invocar una función interna o a la ejecución de procesos de manera externos

char ** command_to_array (scommand cmd){
    //scommand cmd_aux = scommand_copy(cmd);
    unsigned int cmd_size = scommand_length(cmd);
    char ** array = calloc(sizeof(char *), cmd_size + 1);

    
    for(unsigned int i = 0; i < cmd_size; ++i){
        array[i] = strdup(scommand_front(cmd));
        scommand_pop_front(cmd);
    }

    array[cmd_size] = NULL;
    //cmd = scommand_destroy(cmd_aux);

    return array;
}

//redireccion del stdin
static void change_in (scommand cmd){ 
    assert (cmd != NULL);                                  

    char * rdir_in = scommand_get_redir_in(cmd);                 //redireccion que nos interesa

    if (rdir_in != NULL){
        

        int fd_O = open(rdir_in,O_RDONLY, S_IRUSR);             //abrimos el path con permisos solo de lecutra

        if (fd_O < 0){
            perror("Error in open");     
        }

        int fd_D = dup2(fd_O,STDIN_FILENO);                          //redireccionamos al stdin

        if (fd_D < 0){
            perror("Error in redir");
        }          

        int fd_C = close (fd_O);                                     //cerramos lo que abrimos

        if (fd_C < 0){
            perror("Error in close");
        }
    }
    
}

//redireccion del stdout es static 

static void change_out (scommand cmd){                                     

    char * rdir_out = scommand_get_redir_out(cmd);                 //redireccion que nos interesa

    if (rdir_out != NULL){
        
        int fd_O = open(rdir_out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);             //abrimos el path con permisos de escritura

        if (fd_O < 0 ){
            perror("Error in open");       
            }

        int fd_D = dup2(fd_O,STDOUT_FILENO);                          //redireccionamos al stdout

        if (fd_D < 0){
            perror("Error in redir");
           
        }          

        int fd_C = close (fd_O);                                     //cerramos lo que abrimos

        if (fd_C < 0){
            perror("Error in close");
        }
    }
}

//ejecuta comando externo
void execute_external_cmd (scommand cmd){       

    if (scommand_get_redir_in(cmd) != NULL) {   //veo si tiene redir_in
        change_in(cmd);
    }
    if (scommand_get_redir_out(cmd) != NULL) {  //veo si tiene redir_out
        change_out(cmd);
    }
    char ** array_args = command_to_array (cmd);    //array con comando y sus argumentos
                                                    //comando 

    int error = execvp(array_args[0],array_args);        //ejecuta el comando con sus argumentos

    if (error < 0){
        perror ("Error in execvp");
        exit (EXIT_FAILURE);
    }    

}

//diferenciar entre un comando simple y uno multiple
//en el multiple vamos a tener que usar pipe()

void command_simple (scommand cmd){

    if (builtin_is_internal(cmd)){
        
        builtin_run(cmd);

    }else{
        pid_t pid = fork();

        if (pid < 0){
            perror ("Error in fork");

            exit (EXIT_FAILURE);
        }else if (pid == 0){                 //estamos en el hijo

            execute_external_cmd(cmd);

        }else{                               //estamos en el padre;
            //wait(NULL);
        }
    }

}


//        fd_0     fd_1 |f_0   fd_1

void command_double (pipeline apipe){
    int buffer[2];
    int p = pipe(buffer);

    if (p < 0){
        perror("Error in pipe");
    }

    pid_t fork_one = fork();

    if (fork_one < 0){
        perror("Error in fork");
        exit (EXIT_FAILURE);
    }
    else if (fork_one == 0){                                          //estamos en hijo
        close (buffer[STDIN_FILENO]);                                 //cerramos extremo de lectura

        int err_d = dup2 (buffer[1],STDOUT_FILENO);                   //redirige la salida estandar out a la del pipe

        if (err_d < 0){
            perror("Error in dup");
                        exit (EXIT_FAILURE);

        }

        close (buffer[1]);                                              //terminamos de usarlo, lo cerramos
//....................................................................//
        if (builtin_is_internal(pipeline_front(apipe))){
        
        builtin_run(pipeline_front(apipe));

        }else{

        execute_external_cmd(pipeline_front(apipe));
        }

        
    }                        
    else{                                                               //estamo en padre
     pid_t fork_two = fork();
            if(fork_two < 0) {
                perror("Fork second failed");
                exit(EXIT_FAILURE);
            }
            else if (fork_two == 0){    
                pipeline_pop_front(apipe);                              //eliminamos comando que ejecutamos
                close (buffer[STDOUT_FILENO]);                          //cerramos extremo de escritura

                int err_d_2 = dup2 (buffer[0],STDIN_FILENO);            //redireccionamos la entrada in a la del pipe
                if (err_d_2 < 0){
                        perror("Error in dup");
                        exit (EXIT_FAILURE);

                }

                close (buffer[STDIN_FILENO]);                            //terminamos de usarlo, lo cerramos

                if (builtin_is_internal(pipeline_front(apipe))){
        
                builtin_run(pipeline_front(apipe));

                }else{

                execute_external_cmd(pipeline_front(apipe));
                 }
                 //ejecuta segundo comando
            }
            else{                                                        //abuelo
                close (buffer[0]);                                      //no nos interesa comunicar procesos, lo cerramos
                close (buffer[1]);
                if (pipeline_get_wait(apipe)){                          //si esta en background esperamos que los hijos terminen
                    //wait(NULL);
                    wait(NULL);
                }
                
            }

    }

}


void execute_foreground (pipeline apipe){
    
    if (pipeline_length(apipe) != 0){
        if (pipeline_length(apipe) == 1){
            scommand cmd = pipeline_front (apipe);
            command_simple(cmd);
        } else if ( pipeline_length(apipe) == 2){
            command_double(apipe);
        } else{
            perror ("Only execute two commands");
        }
    }
}

void execute_background (pipeline apipe){

    pid_t pid = fork ();

    if (pid == -1){
        perror ("Error in fork() of background");
    }
    else if (pid == 0){
        int buffer[2];

        int p = pipe (buffer);

        if(p == -1){
        perror("Error in pipe");
        exit(EXIT_FAILURE);
        }

        close (buffer[1]);

        int n_dup = dup2 (buffer[0],STDIN_FILENO);

        if (n_dup == -1){
            perror ("Error in dup2");
        }

        execute_foreground(apipe);
        exit (EXIT_SUCCESS);
        
    }
}


void execute_pipeline(pipeline apipe){
    assert (apipe != NULL);

    if (pipeline_get_wait(apipe)){

        execute_foreground(apipe);
        wait (NULL);

    }
    else{
        execute_background(apipe);
    }

}


