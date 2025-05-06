/* Ejecuta comandos simples y pipelines.
 * No toca ningún comando interno.
 */

#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"


void execute_pipeline(pipeline apipe);
/*
 * Ejecuta un pipeline, identificando comandos internos, forkeando, y
 *   redirigiendo la entrada y salida. puede modificar `apipe' en el proceso
 *   de ejecución.
 *   apipe: pipeline a ejecutar
 * Requires: apipe!=NULL
 */


char ** command_to_array (scommand cmd);
void execute_external_cmd (scommand cmd);
void command_simple (scommand cmd);
void command_double (pipeline apipe);
void execute_foreground (pipeline apipe);
void execute_background (pipeline apipe);

#endif /* EXECUTE_H */
