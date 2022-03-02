/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <sys/types.h>
#include "../value.h"

#define VAR_ARG_LEN UINT_MAX

// AR_Func - function pointer to an operation with an arithmetic expression
typedef SIValue(*AR_Func)(SIValue *argv, int argc);

// AR_Func_Finalize - function pointer to a routine for computing an aggregate function's final value
typedef void (*AR_Func_Finalize)(void *ctx);

// AR_Func_Free - function pointer to a routine for freeing a function's private data
typedef void (*AR_Func_Free)(void *ctx);

// AR_Func_Clone - function pointer to a routine for cloning a function's private data
typedef void *(*AR_Func_Clone)(void *orig);

// AR_Func_DefaultValue - function pointer to a routine for producing an aggregation default value
typedef SIValue (*AR_Func_DefaultValue)(void);

// aggregation function callbacks
typedef struct {
	AR_Func_Free free;                  // [optional] function pointer to cleanup routine
	AR_Func_Clone clone;                // [optional] function pointer to clone routine
	AR_Func_Finalize finalize;          // [optional] function pointer to finalizing aggregate value routine
	AR_Func_DefaultValue default_value; // function pointer to get aggregation default value
} AggCBs;

typedef struct {
	AR_Func func;               // function pointer to scalar or aggregate function routine
	SIType *types;              // types of arguments
	uint min_argc;              // minimal number of arguments function expects
	uint max_argc;              // maximal number of arguments function expects
	bool reducible;             // can be reduced using static evaluation
	bool aggregate;             // true if the function is an aggregation
	const char *name;           // function name
	AggCBs agg_callbacks;       // aggregation callbacks
} AR_FuncDesc;

// create a new function descriptor
AR_FuncDesc *AR_FuncDescNew
(
	const char *name,     // function name
	AR_Func func,         // pointer to function
	uint min_argc,        // minimum number of arguments
	uint max_argc,        // maximum number of arguments
	SIType *types,        // acceptable types
	bool reducible        // is function reducible
);

// create a new aggregation function descriptor
AR_FuncDesc *AR_AggFuncDescNew
(
	const char *name,                   // function name
	AR_Func func,                       // pointer to function
	uint min_argc,                      // minimum number of arguments
	uint max_argc,                      // maximum number of arguments
	SIType *types,                      // acceptable types
	AR_Func_Free free,                  // free aggregation callback
	AR_Func_Finalize finalize,          // finalize aggregation callback
	AR_Func_DefaultValue default_value  // default value callback
);

// register arithmetic function to repository
void AR_RegFunc
(
	AR_FuncDesc *func
);

// retrieves an arithmetic function by its name
AR_FuncDesc *AR_GetFunc
(
	const char *func_name
);

// check to see if function exists
// TODO: move this function to more appropriate place
bool AR_FuncExists
(
	const char *func_name
);

// check to see if function is an aggregation
bool AR_FuncIsAggregate
(
	const char *func_name
);

// invoke finalize routine for function
void AR_Finalize
(
	AR_FuncDesc *func_desc
);

// TODO: might need to be removed
// duplicate a function descriptor and populate the clone with the given private data
AR_FuncDesc *AR_SetPrivateData
(
	const AR_FuncDesc *orig,
	void *privdata
);

// TODO: might need to be removed
// clone the given function descriptor and its private data
AR_FuncDesc *AR_CloneFuncDesc
(
	const AR_FuncDesc *orig
);

