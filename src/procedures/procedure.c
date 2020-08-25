/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "procedures.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "rax.h"

static rax *__procedures = NULL;

static void _procRegister(const char *procedure, ProcGenerator gen) {
	raxInsert(__procedures, (unsigned char *)procedure, strlen(procedure), gen, NULL);
}

// Register procedures.
void Proc_Register() {
	__procedures = raxNew();
	_procRegister("db.labels", Proc_LabelsCtx);
	_procRegister("db.propertyKeys", Proc_PropKeysCtx);
	_procRegister("db.relationshipTypes", Proc_RelationsCtx);

	// Register graph algorithms.
	_procRegister("algo.pageRank", Proc_PagerankCtx);
	_procRegister("algo.BFS", Proc_BFS_Ctx);
	_procRegister("algo.BFSTree", Proc_BFSTree_Ctx);

	// Register FullText Search generator.
	_procRegister("db.idx.fulltext.drop", Proc_FulltextDropIdxGen);
	_procRegister("db.idx.fulltext.queryNodes", Proc_FulltextQueryNodeGen);
	_procRegister("db.idx.fulltext.createNodeIndex", Proc_FulltextCreateNodeIdxGen);
}

ProcedureCtx *ProcCtxNew(const char *name,
						 unsigned int argc,
						 ProcedureOutput **output,
						 ProcStep fStep,
						 ProcInvoke fInvoke,
						 ProcFree fFree,
						 void *privateData,
						 bool readOnly) {

	ProcedureCtx *ctx = rm_malloc(sizeof(ProcedureCtx));
	ctx->argc = argc;
	ctx->name = name;
	ctx->Step = fStep;
	ctx->output = output;
	ctx->Invoke = fInvoke;
	ctx->Free = fFree;
	ctx->privateData = privateData;
	ctx->readOnly = readOnly;
	return ctx;
}

ProcedureCtx *Proc_Get(const char *proc_name) {
	if(!__procedures) return NULL;
	ProcGenerator gen = raxFind(__procedures, (unsigned char *)proc_name, strlen(proc_name));
	if(gen == raxNotFound) return NULL;
	ProcedureCtx *ctx = gen();

	// Set procedure state to not initialized.
	ctx->state = PROCEDURE_NOT_INIT;
	return ctx;
}

ProcedureResult Proc_Invoke(ProcedureCtx *proc, const SIValue *args) {
	assert(proc);

	// Procedure is expected to be in the `PROCEDURE_NOT_INIT` state.
	if(proc->state != PROCEDURE_NOT_INIT) {
		proc->state = PROCEDURE_ERROR;
		return PROCEDURE_ERR;
	}

	if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) assert(proc->argc == array_len((SIValue *)args));

	ProcedureResult res = proc->Invoke(proc, args);
	// Set state to initialized.
	if(res == PROCEDURE_OK) proc->state = PROCEDURE_INIT;
	return res;
}

SIValue *Proc_Step(ProcedureCtx *proc) {
	assert(proc);
	// Validate procedure state, can only consumed if state is initialized.
	if(proc->state != PROCEDURE_INIT) return NULL;

	SIValue *val = proc->Step(proc);
	/* Set procedure state to depleted if NULL is returned.
	 * NOTE: we might have errored. */
	if(val == NULL) proc->state = PROCEDURE_DEPLETED;
	return val;
}

uint Procedure_Argc(const ProcedureCtx *proc) {
	assert(proc);
	return proc->argc;
}

uint Procedure_OutputCount(const ProcedureCtx *proc) {
	assert(proc);
	return array_len(proc->output);
}

const char *Procedure_GetOutput(const ProcedureCtx *proc, uint output_idx) {
	assert(proc && output_idx < Procedure_OutputCount(proc));
	return proc->output[output_idx]->name;
}

bool Procedure_ContainsOutput(const ProcedureCtx *proc, const char *output) {
	assert(proc && output);
	uint output_count = array_len(proc->output);
	for(uint i = 0; i < output_count; i++) {
		if(strcmp(proc->output[i]->name, output) == 0) return true;
	}
	return false;
}

bool Proc_ReadOnly(const char *proc_name) {
	assert(__procedures);
	ProcGenerator gen = raxFind(__procedures, (unsigned char *)proc_name, strlen(proc_name));
	if(gen == raxNotFound) return false; // Invalid procedure specified, handled elsewhere.
	/* TODO It would be preferable to be able to determine whether a procedure is read-only
	 * without creating its entire context; this is wasteful. */
	ProcedureCtx *ctx = gen();
	bool read_only = ctx->readOnly;
	Proc_Free(ctx);
	return read_only;
}

void Proc_Free(ProcedureCtx *proc) {
	if(!proc) return;
	proc->Free(proc);

	if(proc->output) {
		for(uint i = 0; i < array_len(proc->output); i++) rm_free(proc->output[i]);
		array_free(proc->output);
	}

	rm_free(proc);
}

