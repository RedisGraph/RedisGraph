/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_procedure_call.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult ProcCallInit(OpBase *opBase);
static Record ProcCallConsume(OpBase *opBase);
static OpResult ProcCallReset(OpBase *opBase);
static void ProcCallFree(OpBase *opBase);

static void _yield(OpProcCall *op, SIValue *proc_output, Record r) {
	if(!op->yield_map) {
		op->yield_map = rm_malloc(sizeof(OutputMap) * array_len(op->output));

		for(uint i = 0; i < array_len(op->output); i++) {
			const char *yield = op->output[i];
			for(uint j = 0; j < array_len(proc_output); j += 2) {
				char *key = (proc_output + j)->stringval;
				if(strcmp(yield, key) == 0) {
					int idx;
					assert(OpBase_Aware((OpBase *)op, key, &idx));
					op->yield_map[i].proc_out_idx = j + 1;
					op->yield_map[i].rec_idx = idx;
					break;
				}
			}
		}
	}

	for(uint i = 0; i < array_len(op->output); i++) {
		int idx = op->yield_map[i].rec_idx;
		uint proc_out_idx = op->yield_map[i].proc_out_idx;
		SIValue *val = proc_output + proc_out_idx;
		// TODO: Migrate this switch into Record.
		Node *n;
		Edge *e;
		switch(val->type) {
		case T_NODE:
			n = val->ptrval;
			Record_AddNode(r, idx, *n);
			break;
		case T_EDGE:
			e = val->ptrval;
			Record_AddEdge(r, idx, *e);
			break;
		default:
			Record_AddScalar(r, idx, *val);
			break;
		}
	}
}

OpBase *NewProcCallOp(const ExecutionPlan *plan, const char *procedure, const char **args,
					  AR_ExpNode **yield_exps) {
	assert(procedure);
	OpProcCall *op = malloc(sizeof(OpProcCall));
	op->args = args;
	op->yield_exps = yield_exps;
	op->procedure = Proc_Get(procedure);
	op->yield_map = NULL;

	uint yield_count = array_len(yield_exps);
	op->output = array_new(const char *, yield_count);

	assert(op->procedure);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROC_CALL, "ProcedureCall", ProcCallInit, ProcCallConsume,
				ProcCallReset, NULL, ProcCallFree, plan);

	// Set modifiers.
	for(uint i = 0; i < yield_count; i ++) {
		const char *alias = yield_exps[i]->resolved_name;
		const char *yield = yield_exps[i]->operand.variadic.entity_alias;

		op->output = array_append(op->output, yield);
		OpBase_Modifies((OpBase *)op, yield);
		if(alias && strcmp(alias, yield) != 0) OpBase_AliasModifier((OpBase *)op, yield, alias);
	}
	if(op->procedure->isWriteProcedure) OpBase_RegisterAsWriter((OpBase *)op);
	return (OpBase *)op;
}

static OpResult ProcCallInit(OpBase *opBase) {
	OpProcCall *op = (OpProcCall *)opBase;
	ProcedureResult res = Proc_Invoke(op->procedure, op->args);
	return (res == PROCEDURE_OK) ? OP_OK : OP_ERR;
}

static Record ProcCallConsume(OpBase *opBase) {
	OpProcCall *op = (OpProcCall *)opBase;
	Record r = NULL;

	if(op->op.childCount == 0) {
		/* Make record large enough to accommodate all alias entities. */
		r = OpBase_CreateRecord((OpBase *)op);
	} else {
		OpBase *child = op->op.children[0];
		r = OpBase_Consume(child);
		if(!r) return NULL;
	}

	SIValue *outputs = Proc_Step(op->procedure);
	if(outputs == NULL) {
		Record_Free(r);
		return NULL;
	}

	// Add procedure outputs to record.
	_yield(op, outputs, r);
	return r;
}

static OpResult ProcCallReset(OpBase *ctx) {
	OpProcCall *op = (OpProcCall *)ctx;
	if(op->procedure) {
		ProcedureResult res = ProcedureReset(op->procedure);
		return (res == PROCEDURE_OK) ? OP_OK : OP_ERR;
	}
	return OP_OK;
}

static void ProcCallFree(OpBase *ctx) {
	OpProcCall *op = (OpProcCall *)ctx;
	if(op->procedure) {
		Proc_Free(op->procedure);
		op->procedure = NULL;
	}

	if(op->yield_map) {
		rm_free(op->yield_map);
		op->yield_map = NULL;
	}

	if(op->args) {
		array_free(op->args);
		op->args = NULL;
	}

	if(op->output) {
		array_free(op->output);
		op->output = NULL;
	}

	if(op->yield_exps) {
		uint yield_count = array_len(op->yield_exps);
		for(uint i = 0; i < yield_count; i ++) AR_EXP_Free(op->yield_exps[i]);
		array_free(op->yield_exps);
		op->yield_exps = NULL;
	}
}
