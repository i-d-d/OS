#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STR_LEN 16
#define ARR_SIZE_INIT 32

typedef enum {CHANGE_IMP, EXEC_1, EXEC_2, EXIT, UNKNOWN} command;

command get_command();
void execute_command(command com);

#endif