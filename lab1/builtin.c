#include <assert.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "builtin.h"
#include "command.h"
#include "strextra.h"
#include "tests/syscall_mock.h"

#define PATH_SIZE 1024

/*Cantidad de comandos internos*/
#define INTERNOS 3

bool exit_bash = false;

/*Estructura de datos con comandos internos*/
char * commands[INTERNOS] = {"cd", "help", "exit"};

/*Funciones bool para run_cd. Aca vemos los caracteres "especiales"; '.' , '/' , '~' , '..'*/
static bool space_char (char *path){
    unsigned int p_length = strlen (path);
    bool is_space = true;
    unsigned int i = 0;
    while (i < p_length && is_space){
        is_space = is_space && path[i] == ' ';
        i++;
    }
    return is_space;
}

static bool is_go_back (char *path, unsigned int p_length){
    return (p_length == 2) && (path[0] == '.' && path[1] == '.');
}

static bool other_special_char (char * path, unsigned int p_length){
    return (p_length == 1 && (path[0] == '.' || path[0] == '~' || path[0] == '/'));
}

static bool special_characters (unsigned int p_length, char * path){
    return (p_length == 2 && space_char (path)) || is_go_back(path, p_length) || other_special_char (path, p_length);
}

void run_cd(const scommand cmd){
    assert (cmd != NULL);

    char * cwd = NULL;
    char * path = NULL;
    bool root_file = false;
    cwd = getcwd (cwd, 1024);

    if (cwd == NULL){
        perror ("Error al obtener el directorio actual");
        exit (EXIT_FAILURE);
    }

    char * go_to_path = dirname (cwd);
    scommand_pop_front (cmd);

    if (scommand_is_empty (cmd)){
        go_to_path = getenv ("HOME");
    }
    else{
        path = scommand_front (cmd);
        unsigned int p_length = strlen (path);
        bool path_is_special =  special_characters(p_length, path);

        if (!path_is_special && path[0] != '/'){
            path = strmerge("/", path);
        }
        else{
            root_file = true;
        }

        if (path_is_special){

            if ((path[0] == '.' && p_length == 1) || p_length == 0){
                go_to_path = getcwd (cwd, PATH_SIZE);
            }
            else if (path[0] == '/'){
                go_to_path = "/";
            }
            else if (is_go_back (path, p_length)){

                if (cwd != NULL){
                    char * cwd_aux = cwd;
                    unsigned int curr_length = strlen (cwd);
                    unsigned int last_slash = curr_length - 1;

                    while (last_slash > 0 && cwd_aux[last_slash] != '/'){
                        last_slash --;
                    }

                    if (last_slash > 0 ){
                        cwd_aux [last_slash] = '\0';
                    }
                    
                    go_to_path = cwd_aux;
                }
                else{
                    go_to_path = "/";
                }
            }
            else{
                go_to_path = getenv ("HOME");
            }

        }
        else{

            if (root_file){
                go_to_path = strdup (path);
            } 
            else{
                cwd = getcwd (cwd, PATH_SIZE);
                char *abs_path = malloc (PATH_SIZE);
                snprintf (abs_path, PATH_SIZE, "%s/%s", cwd, path);
                go_to_path = abs_path;
            }
        }
        if (go_to_path == NULL || strlen (go_to_path) == 0){
            perror ("Error en cd: Ruta de directorio no valida");
        } 
        else{
            int check = chdir(go_to_path);
            
            if (check != 0){
                perror ("Error en cd");
                }
        }
    }  
}

void run_exit (void){
    exit_bash = true;
}

void run_help (const char *aux){
    char * help_str = "\n"
                      "------------------------------\n"
                      "MyBash Wolovers 2023 - Creada por:\n"
                      "Nicolás Fernandez, Guadalupe Galindo, Felipe Jimenez y Fabrizio Longhi\n"
                      "\n"
                      "Comandos Internos:\n"
                      "cd: Permite moverse entre directorios del sistema.\n"
                      "help: Printea información sobre la shell y los comandos internos.\n"
                      "exit: Provoca la terminación del proceso.\n"
                      "------------------------------"
                      "\n";
    
    printf ("%s", help_str);
}

bool builtin_is_internal (scommand cmd){
    assert(cmd != NULL);

    unsigned int i = 0;
    bool result = false;
    char * comando = scommand_front (cmd);
    
    while (!result && i < INTERNOS){
        result = (strcmp (comando, commands[i]) == 0);
        i++;
    }

    return result;
}


bool builtin_alone(pipeline p){
    assert (p != NULL);

    assert (pipeline_length (p) == 1 && builtin_is_internal (pipeline_front (p)));
    return (pipeline_length (p) == 1 && builtin_is_internal (pipeline_front (p)));
}

void builtin_run (scommand cmd){
    assert (builtin_is_internal (cmd));

    char * comando = scommand_front (cmd);
    const char * aux = NULL;

    if (strcmp (comando, commands[0]) == 0){
        run_cd (cmd);
    }
    if (strcmp (comando, commands[1]) == 0){
        run_help (aux);
    }
    if (strcmp (comando, commands[2]) == 0){
        run_exit ();
    }

    comando = NULL;
}
