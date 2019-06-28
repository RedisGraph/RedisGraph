/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../ast/ast.h"
#include "../../procedures/procedure.h"

/* Maps procedure outout to record index.
 * yield element I is mapped to procedure output J
 * which will be stored within Record at position K. */
typedef struct {
    uint proc_out_idx;  // Index into procedure output.
    uint rec_idx;       // Index into record.
} OutputMap;

/* OpProcCall, */
typedef struct {
    OpBase op;                  // Base op.
    AST *ast;                   // AST.
    const char **args;          // Arguments passed to procedure.
    const char **output;        // Procedure output.
    ProcedureCtx *procedure;    // Procedure to call.
    OutputMap *yield_map;       // Maps between yield to procedure output and record idx.
} OpProcCall;

OpBase* NewProcCallOp (
    const char *procedure,    // Procedure name.
    const char **args,        // Arguments passed to procedure invocation.
    const char **output,      // Procedure output.
    uint *modifies            // Record IDs of outputs
);

OpResult OpProcCallInit (
    OpBase *opBase
);

Record OpProcCallConsume (
    OpBase *opBase
);

OpResult OpProcCallReset (
    OpBase *ctx
);

void OpProcCallFree (
    OpBase *ctx
);
