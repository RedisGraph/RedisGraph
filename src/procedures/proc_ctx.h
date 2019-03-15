/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once 

#include "../execution_plan/record.h"

// Procedure response type.
typedef enum {    
    PROCEDURE_OK = 0,
    PROCEDURE_ERR = (1<<0),
} ProcedureResult;

int (*Step)(struct AggCtx *ctx, SIValue *argv, int argc);

struct ProcedureCtx {
    const char *name;       // Procedure name.
    uint argc;              // Number of arguments procedure accepts.
    char **output;          // Procedure possible output(s).
    void *privateData;
    ProcedureResult (*Step)(struct ProcedureCtx *ctx, Record r);
    ProcedureResult (*Invoke)(struct ProcedureCtx *ctx, const char **args);
    ProcedureResult (*Finalize)(struct ProcedureCtx *ctx);
};
typedef struct ProcedureCtx ProcedureCtx;
