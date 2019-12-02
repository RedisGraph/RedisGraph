/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../ast/ast.h"
#include "../execution_plan.h"
#include "../../procedures/procedure.h"

/* Maps procedure output to record index.
 * yield element I is mapped to procedure output J
 * which will be stored within Record at position K. */
typedef struct {
	uint proc_out_idx;  // Index into procedure output.
	uint rec_idx;       // Index into record.
} OutputMap;

/* OpProcCall, */
typedef struct {
	OpBase op;                  // Base op.
	const char **args;          // Arguments passed to procedure.
	const char **output;        // Procedure output.
    AR_ExpNode **yield_exps;    // Yield expressions.
	ProcedureCtx *procedure;    // Procedure to call.
	OutputMap *yield_map;       // Maps between yield to procedure output and record idx.
} OpProcCall;

OpBase *NewProcCallOp(
	const ExecutionPlan *plan,  // Execution plan this operation belongs to.
	const char *procedure,      // Procedure name.
	const char **args,          // Arguments passed to procedure invocation.
	AR_ExpNode **yield_exps         // Procedure output.
);
