/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <sys/types.h>
#include "../value.h"

#define VAR_ARG_LEN UINT_MAX

/* AR_Func - Function pointer to an operation with an arithmetic expression */
typedef SIValue(*AR_Func)(SIValue *argv, int argc);

typedef struct {
	uint min_argc;      // Minimal number of arguments function expects
	uint max_argc;      // Maximal number of arguments function expects
	AR_Func func;       // Function pointer.
	SIType *types;      // Types of arguments.
	const char *name;   // Function name.
	bool reducible;     // Can be reduced using static evaluation.
} AR_FuncDesc;

AR_FuncDesc *AR_FuncDescNew(const char *name, AR_Func func, uint min_argc, uint max_argc,
							SIType *types, bool reducible);

/* Register arithmetic function to repository. */
void AR_RegFunc(AR_FuncDesc *func);

/* Retrieves an arithmetic function by its name. */
AR_FuncDesc *AR_GetFunc(const char *func_name);

/* Check to see if function exists.
 * TODO: move this function to more appropriate place. */
bool AR_FuncExists(const char *func_name);

