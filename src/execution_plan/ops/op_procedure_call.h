/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../ast/ast.h"
#include "../execution_plan.h"
#include "../../procedures/procedure.h"

/* OpProcCall, */
typedef struct {
	OpBase op;                  // Base op.
    uint arg_count;             // Number of arguments.
    AR_ExpNode **arg_exps;      // Expression representing arguments to procedure.
	const char *proc_name;      // Procedure name.
    AR_ExpNode **yield_exps;    // Yield expressions.
} OpProcCall;

OpBase *NewProcCallOp(
	const ExecutionPlan *plan,  // Execution plan this operation belongs to.
	const char *proc_name,      // Procedure name.
    AR_ExpNode **arg_exps,      // Arguments passed to procedure invocation.
	AR_ExpNode **yield_exps     // Procedure output.
);
