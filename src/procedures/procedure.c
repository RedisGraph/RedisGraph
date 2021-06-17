/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "procedures.h"
#include "rax.h"
#include "../RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../util/strutil.h"
#include "../graph/graphcontext.h"

rax *__procedures = NULL;

static void _procRegister(const char *procedure, ProcGenerator gen) {
	char lowercase_proc_name[128];
	size_t lowercase_proc_name_len = 128;
	str_tolower(procedure, lowercase_proc_name, &lowercase_proc_name_len);
	raxInsert(__procedures, (unsigned char *)lowercase_proc_name,
		   lowercase_proc_name_len, gen, NULL);
}

// Register procedures.
void Proc_Register() {
	__procedures = raxNew();
	_procRegister("db.labels", Proc_LabelsCtx);
	_procRegister("db.indexes", Proc_IndexesCtx);
	_procRegister("db.propertyKeys", Proc_PropKeysCtx);
	_procRegister("dbms.procedures", Proc_ProceduresCtx);
	_procRegister("db.relationshipTypes", Proc_RelationsCtx);

	// Register graph algorithms.
	_procRegister("algo.BFS", Proc_BFS_Ctx);
	_procRegister("algo.pageRank", Proc_PagerankCtx);

	// Register FullText Search generator.
	_procRegister("db.idx.fulltext.drop", Proc_FulltextDropIdxGen);
	_procRegister("db.idx.fulltext.queryNodes", Proc_FulltextQueryNodeGen);
	_procRegister("db.idx.fulltext.createNodeIndex", Proc_FulltextCreateNodeIdxGen);
}

ProcedureCtx *ProcCtxNew(const char *name,
						 unsigned int argc,
						 ProcedureOutput *output,
						 ProcStep fStep,
						 ProcInvoke fInvoke,
						 ProcFree fFree,
						 void *privateData,
						 bool readOnly) {

	ProcedureCtx *ctx = rm_malloc(sizeof(ProcedureCtx));
	ctx->argc = argc;
	ctx->name = name;
	ctx->Step = fStep;
	ctx->Free = fFree;
	ctx->output = output;
	ctx->Invoke = fInvoke;
	ctx->privateData = privateData;
	ctx->readOnly = readOnly;
	return ctx;
}

ProcedureCtx *Proc_Get(const char *proc_name) {
	if(!__procedures) return NULL;
	size_t proc_name_len = strlen(proc_name) + 1;
	char proc_name_lowercase [proc_name_len];
	str_tolower(proc_name, proc_name_lowercase, &proc_name_len);
	ProcGenerator gen = raxFind(__procedures, (unsigned char *)proc_name_lowercase,
	  			proc_name_len);
	if(gen == raxNotFound) return NULL;
	ProcedureCtx *ctx = gen();

	// Set procedure state to not initialized.
	ctx->state = PROCEDURE_NOT_INIT;
	return ctx;
}

ProcedureResult Proc_Invoke(ProcedureCtx *proc, const SIValue *args, const char **yield) {
	ASSERT(proc != NULL);
	// Procedure is expected to be in the `PROCEDURE_NOT_INIT` state.
	if(proc->state != PROCEDURE_NOT_INIT) {
		proc->state = PROCEDURE_ERROR;
		return PROCEDURE_ERR;
	}

	if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) {
		uint argc = array_len((SIValue *)args);
		ASSERT(proc->argc == argc);
	}

	ProcedureResult res = proc->Invoke(proc, args, yield);
	// Set state to initialized.
	if(res == PROCEDURE_OK) proc->state = PROCEDURE_INIT;
	return res;
}

SIValue *Proc_Step(ProcedureCtx *proc) {
	ASSERT(proc != NULL);
	// Validate procedure state, can only consumed if state is initialized.
	if(proc->state != PROCEDURE_INIT) return NULL;

	SIValue *val = proc->Step(proc);
	/* Set procedure state to depleted if NULL is returned.
	 * NOTE: we might have errored. */
	if(val == NULL) proc->state = PROCEDURE_DEPLETED;
	return val;
}

uint Procedure_Argc(const ProcedureCtx *proc) {
	ASSERT(proc != NULL);
	return proc->argc;
}

uint Procedure_OutputCount(const ProcedureCtx *proc) {
	ASSERT(proc != NULL);
	return array_len(proc->output);
}

const char *Procedure_GetOutput(const ProcedureCtx *proc, uint output_idx) {
	ASSERT(proc != NULL);
	ASSERT(output_idx < Procedure_OutputCount(proc));
	return proc->output[output_idx].name;
}

bool Procedure_ContainsOutput(const ProcedureCtx *proc, const char *output) {
	ASSERT(proc != NULL);
	ASSERT(output != NULL);
	uint output_count = array_len(proc->output);
	for(uint i = 0; i < output_count; i++) {
		if(strcmp(proc->output[i].name, output) == 0) return true;
	}
	return false;
}

const char *Procedure_GetName(const ProcedureCtx *proc) {
	ASSERT(proc != NULL);
	return proc->name;
}

bool Procedure_IsReadOnly(const ProcedureCtx *proc) {
	ASSERT(proc != NULL);
	return proc->readOnly;
}

void Proc_Free(ProcedureCtx *proc) {
	if(!proc) return;
	proc->Free(proc);

	if(proc->output) array_free(proc->output);

	rm_free(proc);
}

