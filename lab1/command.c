#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "strextra.h"

struct scommand_s{
    GSList * list;
    char * out;
    char * in;
}; 

struct pipeline_s{
    GSList * cmd;
    bool wait;
};

scommand scommand_new (void){
    scommand self = malloc (sizeof (struct scommand_s));

    if (self == NULL){
        perror("Error al reservar memoria");
        exit(EXIT_FAILURE);
    }
    
    self->list = NULL;
    self-> out = NULL;
    self->in = NULL;
    assert(self != NULL && scommand_is_empty(self) && 
    scommand_get_redir_in (self) == NULL && scommand_get_redir_out (self) == NULL);
    return self;
}

scommand scommand_destroy (scommand self){
    assert (self != NULL);

    if (self->out != NULL){
        free (self->out);
        self->out = NULL;
    } 
    if (self->in != NULL){
        free (self->in);
        self->in = NULL;
    }

    g_slist_free_full (self->list,free);
    self->list = NULL;
    free (self);
    self = NULL;
    assert (self == NULL);
    return self;
}

void scommand_push_back (scommand self, char * argument){
    assert (self != NULL && argument != NULL);

    self->list = g_slist_append (self->list, argument);
    assert(!scommand_is_empty(self));
}

void scommand_pop_front (scommand self){
    assert (self != NULL && !scommand_is_empty (self));

    gpointer head = g_slist_nth_data (self->list,0);
    self->list = g_slist_remove (self->list, head);
    free (head);
}

void scommand_set_redir_in (scommand self, char * filename){
    assert (self != NULL);
    if (self->in != NULL){
        free (self->in);
        self->in = filename;
    }
    else {
       self->in = filename; 
    }
}

void scommand_set_redir_out (scommand self, char * filename){
    assert (self != NULL);
    if(self->out != NULL) {
        free(self->out);
        self->out = filename;
    }
    else {
        self->out = filename;
    }
}

bool scommand_is_empty (const scommand self){
    assert (self != NULL);

    bool is_empty = g_slist_length (self->list) == 0u; 
    return is_empty; 
}

unsigned int scommand_length (const scommand self){
    assert (self != NULL);

    unsigned int length = g_slist_length (self->list);
    assert ((length == 0) == scommand_is_empty (self));
    return length;
}

char * scommand_front (const scommand self){
    assert (self != NULL && !scommand_is_empty(self));

    char * first = g_slist_nth_data (self->list, 0u);
    assert (first != NULL);
    return first;
}

char * scommand_get_redir_in (const scommand self){
    assert (self != NULL);

    char * res = NULL;
    res = self->in;
    return res;
}

char * scommand_get_redir_out (const scommand self){
    assert (self != NULL);

    char * res = NULL;
    res = self->out;
    return res;
}

//Modularizacion para scommand_to_string
char * concat_string (char * s1, char * s2){
    char * tmp_s1 = s1; 
    char * tmp_s2 = s2; 
    char * result = strmerge (tmp_s1, tmp_s2);
    return result;    
}

char * scommand_to_string (const scommand self){
    assert ( self != NULL);

    char * result = strdup ("");

    for (unsigned int i = 0; i < scommand_length (self); ++i){
        char * aux = g_slist_nth_data(self->list, i);
        char * tmp = result; 
        result = strmerge (tmp, aux);
        free (tmp);
    }

    if (self->out != NULL){
        char * temp_2 = concat_string (result, ">");   
        free(result);                               //Esto generaba leaks, no liberabamos lo anterior
        result = concat_string (temp_2, self->out);
        free (temp_2);
    }

    if (self->in != NULL){
        char * temp_2 = concat_string (result, "<");   
        free (result);
        result = concat_string (temp_2, self->in);
        free (temp_2);
    }    

    assert (scommand_is_empty (self) ||
    scommand_get_redir_in (self) == NULL || scommand_get_redir_out (self) == NULL ||
    strlen (result) > 0);
    return result;
}

static void scommand_destroy_pipeline (void *self) {
    scommand self_to_destroy = self;
    scommand_destroy (self_to_destroy);
}

//-----------------------------------------------------------------

pipeline pipeline_new (void){
    pipeline self = malloc (sizeof (struct pipeline_s));

    if (self == NULL){
        perror ("Error al reservar memoria");
        exit (EXIT_FAILURE);
    }
    
    self->cmd = NULL;
    self->wait = true;
    assert ((self != NULL) && 
    pipeline_is_empty (self) && pipeline_get_wait (self));
    return self;
}

pipeline pipeline_destroy (pipeline self){
    assert (self != NULL);

    g_slist_free_full (self->cmd, scommand_destroy_pipeline);
    self->cmd = NULL;
    free(self);
    self = NULL;
    assert (self == NULL);
    return self;
}

void pipeline_push_back (pipeline self, scommand sc){
    assert (self != NULL && sc != NULL); 

    self->cmd = g_slist_append (self->cmd, sc);
    assert (!pipeline_is_empty (self));
}

void pipeline_pop_front (pipeline self){
    assert(self != NULL && !pipeline_is_empty (self));
    gpointer head = g_slist_nth_data (self->cmd, 0);
    self->cmd = g_slist_remove (self->cmd, head);
    free(head);
    }

void pipeline_set_wait (pipeline self, const bool w){
    assert (self != NULL);

    self->wait = w;
}

bool pipeline_is_empty (const pipeline self){
    assert (self != NULL);

    bool is_empty = g_slist_length (self->cmd) == 0;
    return is_empty;
}

unsigned int pipeline_length (const pipeline self){
    assert (self != NULL);

    unsigned int length = g_slist_length (self->cmd);
    assert ((length == 0) == pipeline_is_empty (self));
    return length;
}

scommand pipeline_front (const pipeline self){
    assert (self != NULL && !pipeline_is_empty (self));

    scommand result = g_slist_nth_data (self->cmd, 0);
    assert (result != NULL);

    return result;
}

bool pipeline_get_wait (const pipeline self){
    assert (self != NULL);

    return self->wait;
}

char * pipeline_to_string (const pipeline self){
    assert (self != NULL);

    char * result = strdup ("");

    for (unsigned int i = 0; i < pipeline_length (self); ++i){
        char * aux = scommand_to_string (g_slist_nth_data (self->cmd, i));
        char * tmp = result; 
        result = strmerge (tmp,aux);
        free(tmp);

        if (i != g_slist_length (self->cmd)-1){
            //result = strmerge (tmp,"|");
            char *temp_pipe = strmerge(result,"|");
            free(result);
            result = temp_pipe;
        }
        free (aux);
    }  

    if (self->wait == false){
        char * temp_1 = strmerge (result,"&");
        free(result);                               //Faltaba liberar result ya que quedo todo en temp_1
        result = temp_1;
    } 

    assert(pipeline_is_empty (self) || pipeline_get_wait (self) || strlen (result) > 0);
    return result;
}

//----------------------------FUNCION AUXILIAR PARA EL EXCUTE ------------------------
pipeline pipeline_copy (pipeline self){
    pipeline copy = pipeline_new();

    if (self != NULL){
        copy->cmd = g_slist_copy (self->cmd);
        copy->wait = self->wait;
    }

    return copy;
}

scommand scommand_copy (scommand self) {
    scommand copy = scommand_new();

    if (self != NULL){
        GSList *original_list = self->list;
        
        while (original_list != NULL){
            char *original_command = (char *)original_list->data;
            char *copied_command = strdup (original_command);
            copy->list = g_slist_append (copy->list, copied_command);
            original_list = original_list->next;
        }

        copy->out = self->out;
        copy->in = self->in;
    }

    return copy;
}
