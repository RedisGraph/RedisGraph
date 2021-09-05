/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../ast/ast.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_procedure_call.h"
#include "../../../../procedures/procedure.h"

/* Maps procedure output to record index.
 * yield element I is mapped to procedure output J
 * which will be stored within Record at position K. */
typedef struct {
	uint proc_out_idx;  // Index into procedure output.
	uint rec_idx;       // Index into record.
} OutputMap;

/* OpProcCall, */
typedef struct {
	RT_OpBase op;               // Base op.
	const OpProcCall *op_desc;
	AR_ExpNode **arg_exps;      // Expression representing arguments to procedure.
    AR_ExpNode **yield_exps;    // Yield expressions.
    Record r;                   // Current record.
    SIValue *args;              // Computed arguments.
	const char **output;        // Procedure output.
	ProcedureCtx *procedure;    // Procedure to call.
	OutputMap *yield_map;       // Maps between yield to procedure output and record idx.
    bool first_call;            // Indicate first call.
} RT_OpProcCall;

RT_OpBase *RT_NewProcCallOp(const RT_ExecutionPlan *plan, const OpProcCall *op_desc);
