/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "procedures.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../../deps/rax/rax.h"

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

	// Register FullText Search generator.
	_procRegister("db.index.fulltext.drop", Proc_FulltextDropIdxGen);
	_procRegister("db.idx.fulltext.queryNodes", Proc_FulltextQueryNodeGen);
	_procRegister("db.idx.fulltext.createNodeIndex", Proc_FulltextCreateNodeIdxGen);
}

ProcedureCtx *ProcCtxNew(const char *name,
						 unsigned int argc,
						 ProcedureOutput **output,
						 ProcStep fStep,
						 ProcInvoke fInvoke,
						 ProcFree fFree,
						 void *privateData) {

	ProcedureCtx *ctx = rm_malloc(sizeof(ProcedureCtx));
	ctx->argc = argc;
	ctx->name = name;
	ctx->Step = fStep;
	ctx->output = output;
	ctx->Invoke = fInvoke;
	ctx->Free = fFree;
	ctx->privateData = privateData;
	return ctx;
}

ProcedureCtx *Proc_Get(const char *proc_name) {
	if(!__procedures) return NULL;
	ProcGenerator gen = raxFind(__procedures, (unsigned char *)proc_name, strlen(proc_name));
	if(gen == raxNotFound) return NULL;
	ProcedureCtx *ctx = gen();
	return ctx;
}

ProcedureResult Proc_Invoke(ProcedureCtx *proc, const char **args) {
	assert(proc);
	if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) assert(proc->argc == array_len(args));
	// TODO: procedure can only be invoke once.
	return proc->Invoke(proc, args);
}

SIValue *Proc_Step(ProcedureCtx *proc) {
	assert(proc);
	return proc->Step(proc);
}

ProcedureResult ProcedureReset(ProcedureCtx *proc) {
	// return proc->restart(proc);
	return PROCEDURE_OK;
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

void Proc_Free(ProcedureCtx *proc) {
	if(!proc) return;
	proc->Free(proc);
	for(uint i = 0; i < array_len(proc->output); i++) {
		rm_free(proc->output[i]);
	}
	if(proc->output) array_free(proc->output);
	rm_free(proc);
}
