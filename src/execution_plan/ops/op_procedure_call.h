/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../parser/ast.h"
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
	char **args;                // Arguments passed to procedure.
	char **output;              // Procedure output.
	ProcedureCtx *procedure;    // Procedure to call.
	OutputMap *yield_map;       // Maps between yield to procedure output and record idx.
} OpProcCall;

OpBase *NewProcCallOp(
    char *procedure,    // Procedure name.
    char **args,        // Arguments passed to procedure invocation.
    char **output,      // Procedure output.
    AST *ast            // AST.
);

OpResult OpProcCallInit(
    OpBase *opBase
);

Record OpProcCallConsume(
    OpBase *opBase
);

OpResult OpProcCallReset(
    OpBase *ctx
);

void OpProcCallFree(
    OpBase *ctx
);
