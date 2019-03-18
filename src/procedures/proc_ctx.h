/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once 

#include "../value.h"

// Procedure accepts a variable number of arguments.
#define PROCEDURE_VARIABLE_ARG_COUNT UINT_MAX

// Procedure response type.
typedef enum {    
    PROCEDURE_OK = 0,
    PROCEDURE_ERR = (1<<0),
} ProcedureResult;

struct ProcedureCtx;

// Procedure instance generator.
typedef struct ProcedureCtx* (*ProcGenerator)();
// Procedure step function.
typedef SIValue* (*ProcStep)(struct ProcedureCtx *ctx);
// Procedure function pointer.
typedef ProcedureResult (*ProcInvoke)(struct ProcedureCtx *ctx, char **args);
// Procedure free resources.
typedef ProcedureResult (*ProcFree)(struct ProcedureCtx *ctx);

/* ProcedureCtx */
struct ProcedureCtx {
    const char *name;       // Procedure name.
    unsigned int argc;      // Number of arguments procedure accepts.
    char **output;          // Procedure possible output(s).
    void *privateData;      // 
    ProcStep Step;          // 
    ProcInvoke Invoke;      // 
    ProcFree Free;          // 
};
typedef struct ProcedureCtx ProcedureCtx;

ProcedureCtx* ProcCtxNew (
    const char *name,       // Procedure name.
    unsigned int argc,      // Procedure arguments.
    char **output,          // Procedure output.
    ProcStep fStep,         // Procedure Step function.
    ProcInvoke fInvoke,     // Procedure Invoke function.
    ProcFree fFree,         // Procedure Free function.
    void *privateData       // Procedure private data.
);
