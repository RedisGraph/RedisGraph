/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_procedure_call.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

static void _yield(OpProcCall *op, SIValue *proc_output, Record r) {
	if(!op->yield_map) {
		op->yield_map = rm_malloc(sizeof(OutputMap) * array_len(op->output));

		for(uint i = 0; i < array_len(op->output); i++) {
			const char *yield = op->output[i];
			for(uint j = 0; j < array_len(proc_output); j += 2) {
				char *key = (proc_output + j)->stringval;
				SIValue *val = &proc_output[j + 1];
				if(strcmp(yield, key) == 0) {
					OpBase *base = (OpBase *)op;
					int idx = RecordMap_LookupAlias(base->record_map, key);
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

OpBase *NewProcCallOp(const char *procedure, const char **args, const char **output,
					  uint *modifies) {
	assert(procedure);
	OpProcCall *op = malloc(sizeof(OpProcCall));
	op->args = args;
	op->output = output;
	op->procedure = Proc_Get(procedure);
	op->yield_map = NULL;

	assert(op->procedure);

	// Set our Op operations
	OpBase_Init(&op->op);
	op->op.name = "ProcedureCall";
	op->op.type = OPType_PROC_CALL;
	op->op.init = OpProcCallInit;
	op->op.consume = OpProcCallConsume;
	op->op.reset = OpProcCallReset;
	op->op.free = OpProcCallFree;

	int outputs_count = array_len(output);
	op->op.modifies = modifies;

	return (OpBase *)op;
}

OpResult OpProcCallInit(OpBase *opBase) {
	OpProcCall *op = (OpProcCall *)opBase;
	ProcedureResult res = Proc_Invoke(op->procedure, op->args);
	return (res == PROCEDURE_OK) ? OP_OK : OP_ERR;
}

Record OpProcCallConsume(OpBase *opBase) {
	OpProcCall *op = (OpProcCall *)opBase;
	Record r = NULL;

	if(op->op.childCount == 0) {
		/* Make record large enough to accommodate all alias entities. */
		r = Record_New(opBase->record_map->record_len);
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

OpResult OpProcCallReset(OpBase *ctx) {
	OpProcCall *op = (OpProcCall *)ctx;
	if(op->procedure) {
		ProcedureResult res = ProcedureReset(op->procedure);
		return (res == PROCEDURE_OK) ? OP_OK : OP_ERR;
	}
	return OP_OK;
}

void OpProcCallFree(OpBase *ctx) {
	OpProcCall *op = (OpProcCall *)ctx;
	if(op->args) array_free(op->args);
	if(op->output) array_free(op->output);
	if(op->procedure) Proc_Free(op->procedure);
	if(op->yield_map) rm_free(op->yield_map);
}
