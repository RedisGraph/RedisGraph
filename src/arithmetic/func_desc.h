/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <sys/types.h>
#include "../value.h"

#define VAR_ARG_LEN UINT8_MAX

/* AR_Func - Function pointer */
typedef SIValue(*AR_Func)(SIValue *argv, int argc);

typedef struct {
    AR_Func func;
    uint argc;      // Number of arguments function expects
    SIType *types;  // Types of arguments.
} AR_FuncDesc;

AR_FuncDesc *AR_FuncDescNew(AR_Func func, uint argc, SIType *types);

/* Register arithmetic function to repository. */
void AR_RegFunc(char *func_name, AR_FuncDesc *func);

/* Retrieves an arithmetic function by its name. */
AR_FuncDesc *AR_GetFunc(const char *func_name);

/* Check to see if function exists. 
 * TODO: move this function to more appropriate place. */
bool AR_FuncExists(const char *func_name);
