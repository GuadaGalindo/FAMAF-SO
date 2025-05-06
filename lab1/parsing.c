#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "command.h"
#include "parser.h"
#include "parsing.h"

static scommand parse_scommand (Parser p){
    assert ( p != NULL );

    scommand new_cmd = scommand_new ();
    arg_kind_t arg_type;
    char * aux = parser_next_argument (p, &arg_type);

    while (aux != NULL){                                            //recorremos hasta el primer |
        
        if (arg_type == ARG_NORMAL){                                //si es normal, lo agregamos
            scommand_push_back (new_cmd, aux);
        }
        else if (arg_type == ARG_INPUT){                            //redireccion stdin
            scommand_set_redir_in (new_cmd, aux);
        }
        else if (arg_type == ARG_OUTPUT){                           //redireccion stdout
            scommand_set_redir_out (new_cmd, aux);
        }
 
        aux = NULL;
        aux  = parser_next_argument (p, &arg_type);                  //parseo siguiente comando u argumento

    }

    if (scommand_is_empty (new_cmd)){                                //si es vacio
        new_cmd = scommand_destroy (new_cmd);
        new_cmd = NULL;
    }

    return new_cmd;
}

/* Devuelve NULL cuando hay un error de parseo */

pipeline parse_pipeline (Parser p){
    assert (p!= NULL && !parser_at_eof (p));

    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe = true, is_backg, gargb;
    cmd = parse_scommand (p);
    error = (cmd == NULL);                      //Comando inválido al empezar

    while (another_pipe && !error){

        pipeline_push_back (result, cmd);       //parseo del primer comando simple
        parser_op_pipe (p, &another_pipe);      //veo si hay un pipe

        if (another_pipe){                      //si hay pipe
            cmd = parse_scommand (p);           //parseo siguiente comando
            error = cmd == NULL;                //corrobora si el comando es valido
        }
    }
    
    parser_op_background(p, &is_backg);

    if (is_backg && !error && result != NULL){                   //comando en background
        pipeline_set_wait (result, !is_backg);
    }

    parser_garbage(p, &gargb);                  // eliminamos todo porque ya parseamos

    if (error){                                 //si existe un error, destruimos
        result = pipeline_destroy(result);
    }

    return result;
}

    /* Tolerancia a espacios posteriores */
    /* Consumir todo lo que hay inclusive el \n */
    /* Si hubo error, hacemos cleanup */



