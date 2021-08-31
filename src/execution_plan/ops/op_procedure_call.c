/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_procedure_call.h"
#include "../../RG.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static void ProcCallFree(OpBase *opBase);

OpBase *NewProcCallOp(const ExecutionPlan *plan, const char *proc_name, AR_ExpNode **arg_exps,
					  AR_ExpNode **yield_exps) {

	ASSERT(proc_name != NULL);

	OpProcCall *op = rm_malloc(sizeof(OpProcCall));
	op->arg_exps = arg_exps;
	op->proc_name = proc_name;
	op->yield_exps = yield_exps;
	op->arg_count = array_len(arg_exps);

	// Procedure must exist
	ProcedureCtx *procedure = Proc_Get(proc_name);
	ASSERT(procedure != NULL);

	uint yield_count = array_len(yield_exps);

	// Set operations
	OpBase_Init((OpBase *)op, OPType_PROC_CALL, "ProcedureCall", ProcCallFree,
		!Procedure_IsReadOnly(procedure), plan);

	// Set modifiers
	for(uint i = 0; i < yield_count; i ++) {
		const char *alias = yield_exps[i]->resolved_name;
		const char *yield = yield_exps[i]->operand.variadic.entity_alias;

		OpBase_Modifies((OpBase *)op, yield);
		if(alias && strcmp(alias, yield) != 0) OpBase_AliasModifier((OpBase *)op, yield, alias);
	}

	Proc_Free(procedure);

	return (OpBase*)op;
}

static void ProcCallFree(OpBase *ctx) {
	OpProcCall *op = (OpProcCall *)ctx;
	if(op->arg_exps) {
		for(uint i = 0; i < op->arg_count; i++) AR_EXP_Free(op->arg_exps[i]);
		array_free(op->arg_exps);
		op->arg_exps = NULL;
	}

	if(op->yield_exps) {
		uint yield_count = array_len(op->yield_exps);
		for(uint i = 0; i < yield_count; i ++) AR_EXP_Free(op->yield_exps[i]);
		array_free(op->yield_exps);
		op->yield_exps = NULL;
	}
}
