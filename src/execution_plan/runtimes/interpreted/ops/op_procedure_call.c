/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_procedure_call.h"
#include "../../../../RG.h"
#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"
#include "../../../../query_ctx.h"

/* Forward declarations. */
static Record ProcCallConsume(RT_OpBase *opBase);
static RT_OpResult ProcCallReset(RT_OpBase *opBase);
static void ProcCallFree(RT_OpBase *opBase);

static void _construct_output_mappings(RT_OpProcCall *op, SIValue *outputs) {
	// Map procedure outputs to record indices.
	uint n = array_len(op->output);
	uint m = array_len(outputs);
	op->yield_map = rm_malloc(sizeof(OutputMap) * n);

	for(uint i = 0; i < n; i++) {
		const char *output = op->output[i];
		/* Procedure output is a key/value pair
		 * where the key is at even position and the value is at
		 * an odd position. */
		uint j = 0;
		for(; j < m; j += 2) {
			char *key = (outputs + j)->stringval;
			if(strcmp(output, key) == 0) {
				uint idx;
				bool aware = RT_OpBase_Aware((RT_OpBase *)op, key, &idx);
				UNUSED(aware);
				ASSERT(aware == true);
				op->yield_map[i].proc_out_idx = j + 1;
				op->yield_map[i].rec_idx = idx;
				break;
			}
		}
		// Make sure output was mapped.
		ASSERT(j < m);
	}
}

static Record _yield(RT_OpProcCall *op) {
	SIValue *outputs = Proc_Step(op->procedure);
	if(outputs == NULL) return NULL;

	if(!op->yield_map) _construct_output_mappings(op, outputs);

	Record clone = RT_OpBase_CloneRecord(op->r);
	for(uint i = 0; i < array_len(op->output); i++) {
		int idx = op->yield_map[i].rec_idx;
		uint proc_out_idx = op->yield_map[i].proc_out_idx;
		SIValue val = outputs[proc_out_idx];
		Record_Add(clone, idx, val);
	}

	return clone;
}

static void _evaluate_proc_args(RT_OpProcCall *op) {
	// Evaluate arguments, free args from previous call.
	uint arg_count = array_len(op->args);
	for(uint i = 0; i < arg_count; i++) SIValue_Free(op->args[i]);

	array_clear(op->args);

	for(uint i = 0; i < op->op_desc->arg_count; i++) {
		array_append(op->args, AR_EXP_Evaluate(op->op_desc->arg_exps[i], op->r));
	}
}

RT_OpBase *RT_NewProcCallOp(const RT_ExecutionPlan *plan, const OpProcCall *op_desc) {

	ASSERT(proc_name != NULL);

	RT_OpProcCall *op = rm_malloc(sizeof(RT_OpProcCall));
	op->op_desc = op_desc;
	op->r = NULL;
	op->yield_map = NULL;
	op->first_call = true;
	op->args = array_new(SIValue, op_desc->arg_count);

	// Procedure must exist
	op->procedure = Proc_Get(op_desc->proc_name);
	ASSERT(op->procedure != NULL);

	uint yield_count = array_len(op_desc->yield_exps);
	op->output = array_new(const char *, yield_count);

	// Set operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL,
		ProcCallConsume, ProcCallReset, ProcCallFree, plan);

	// Set modifiers
	for(uint i = 0; i < yield_count; i ++) {
		const char *alias = op_desc->yield_exps[i]->resolved_name;
		const char *yield = op_desc->yield_exps[i]->operand.variadic.entity_alias;

		array_append(op->output, yield);
	}

	return (RT_OpBase*)op;
}

static Record ProcCallConsume(RT_OpBase *opBase) {
	RT_OpProcCall *op = (RT_OpProcCall*)opBase;

	Record yield_record = NULL;
	while(!(yield_record = _yield(op))) {
		// Free old record.
		if(op->r) {
			RT_OpBase_DeleteRecord(op->r);
			op->r = NULL;
		}

		if(op->op.childCount == 0) {
			// "Static evaluation", return data for first call only!
			if(!op->first_call) return NULL;
			op->first_call = false;
			op->r = RT_OpBase_CreateRecord((RT_OpBase *)op);
		} else {
			RT_OpBase *child = op->op.children[0];
			op->r = RT_OpBase_Consume(child);
			if(op->r == NULL) return NULL;
		}

		_evaluate_proc_args(op);

		/* Free previous invocation.
		 * TODO: replace with Proc_Reset */
		Proc_Free(op->procedure);
		op->procedure = Proc_Get(op->op_desc->proc_name);

		// at the moment the only two procedures that can modify the graph are:
		// proc_fulltext_create_index
		// proc_fulltext_drop_index
		// both perform the modification once invoked without returning any
		// additional data (consume/step) function
		// this is why acquiring the write lock as we do below works
		// we will have to revisit this logic once new "write" procedures are
		// introduced

		// lock if procedure can modify the graph
		if(!Procedure_IsReadOnly(op->procedure)) QueryCtx_LockForCommit();

		ProcedureResult res = Proc_Invoke(op->procedure, op->args, op->output);

		// unlock if procedure can modify the graph
		if(!Procedure_IsReadOnly(op->procedure)) QueryCtx_UnlockCommit(opBase);

		/* TODO: should rise run-time exception?
		 * op->r will be freed in ProcCallFree. */
		if(res != PROCEDURE_OK) return NULL;
	}

	return yield_record;
}

static RT_OpResult ProcCallReset(RT_OpBase *ctx) {
	RT_OpProcCall *op = (RT_OpProcCall *)ctx;
	op->first_call = true;
	return OP_OK;
}

static void ProcCallFree(RT_OpBase *ctx) {
	RT_OpProcCall *op = (RT_OpProcCall *)ctx;

	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}

	if(op->procedure) {
		Proc_Free(op->procedure);
		op->procedure = NULL;
	}

	if(op->yield_map) {
		rm_free(op->yield_map);
		op->yield_map = NULL;
	}

	if(op->args) {
		uint arg_count = array_len(op->args);
		for(uint i = 0; i < arg_count; i++) SIValue_Free(op->args[i]);
		array_free(op->args);
		op->args = NULL;
	}

	if(op->output) {
		array_free(op->output);
		op->output = NULL;
	}
}
