#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"


/* Inicializacion global de colores del prompt */
#define ANSI_COLOR_RESET   "\x1b[0m"
char COLOR_TEXT[32] = "\x1b[0m";    /* Colores del texto */
char COLOR_HOST[32] = "\033[38;2;255;51;51m";  /* Colores pertenecientes al host */
char COLOR_PATH[32] = "\033[38;2;153;204;255m"; /* Colores del path */


/* Funcion que muestra el prompt */
void show_prompt(void);
void show_prompt(void) {
    char cwd[1024]; /* Creamos un string para el path */
    getcwd(cwd, 1024);  /* Esta funcion nos devuelve el path y la guarda en el array cwd */

    char host[256];     /* Creamos un string para el host */
    gethostname(host, 256);   /* Esta funcion nos devuelve el nombre del host y 
                                            lo guarda en el string host */

    char user[256];    /* Creamos un string para el user */
    getlogin_r(user, 256); /* Esta funcion nos devuelve el usuario loggeado y 
                                            lo guarda en el string user */


    printf ("%smybash %s%s@%s:%s %s> %s",COLOR_TEXT ,COLOR_HOST,user, host, COLOR_PATH ,cwd, COLOR_TEXT); /* Printf que se muestra en cada ejecucion de pipe */
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    exit_bash = false;
    bool quit = false;

    input = parser_new(stdin);
    while (!exit_bash && !quit) {
        show_prompt();
        pipe = parse_pipeline(input);
        if (pipe == NULL)
        {
            perror("Error in pipe");
            exit(EXIT_FAILURE);
        }
        else
        {
            execute_pipeline(pipe);
            pipe = pipeline_destroy(pipe);
        }
        // Hay que salir luego de ejecutar? //
        quit = parser_at_eof(input);
    }

    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}

