/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once 

#include "../value.h"

// Procedure response type.
typedef enum {    
    PROCEDURE_OK = 0,
    PROCEDURE_ERR = (1<<0),
} ProcedureResult;

/*
 * Finalize - free private data. */
struct ProcedureCtx {
    const char *name;       // Procedure name.
    unsigned int argc;      // Number of arguments procedure accepts.
    char **output;          // Procedure possible output(s).
    void *privateData;
    SIValue* (*Step)(struct ProcedureCtx *ctx);
    ProcedureResult (*Invoke)(struct ProcedureCtx *ctx, char **args);
    ProcedureResult (*Free)(struct ProcedureCtx *ctx);
};
typedef struct ProcedureCtx ProcedureCtx;
