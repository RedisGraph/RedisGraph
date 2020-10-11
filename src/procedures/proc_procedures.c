/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_procedures.h"
#include "../util/arr.h"
#include "rax.h"
#include "procedure.h"

// CALL dbms.procedures()

typedef struct {
	raxIterator *current_proc; // Current proc
	SIValue *output; // Array with a maximum of 4 entries: ["name", name, "mode", mode].
} ProceduresContext;

ProcedureResult Proc_ProceduresInvoke(ProcedureCtx *ctx,
  const SIValue *args, const char **yield) {
	if(array_len((SIValue *)args) != 0) return PROCEDURE_ERR;

	ProceduresContext *pdata = rm_malloc(sizeof(ProceduresContext));

	// Initialize an iterator to the rax that contains all procedures
	rax* rax = Proc_Get_All();
	raxIterator *it = rm_malloc(sizeof(raxIterator));
	raxStart(it, rax);
	raxSeek(it, "^", NULL, 0);
	pdata->current_proc = it;
	pdata->output = array_new(SIValue, 4);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("name"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("mode"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

SIValue *Proc_ProceduresStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	ProceduresContext *pdata = (ProceduresContext *)ctx->privateData;
	raxIterator *it = pdata->current_proc;
	// Depleted?
	if(!raxNext(it)) return NULL;

	// Returns the current procedure's name and mode.
	ProcGenerator gen = it->data;
	ProcedureCtx *curr_proc_ctx = gen();
	pdata->output[1] = SI_DuplicateStringVal(Procedure_GetName(curr_proc_ctx));
	pdata->output[3] = Procedure_IsReadOnly(curr_proc_ctx) ?
	  SI_ConstStringVal("READ") : SI_ConstStringVal("WRITE");
	Proc_Free(curr_proc_ctx);
	return pdata->output;
}

ProcedureResult Proc_ProceduresFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		ProceduresContext *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(pdata->current_proc);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_ProceduresCtx() {
	void *privateData = NULL;

	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput out_name = {name: "name", type: T_STRING};
	ProcedureOutput out_mode = {name: "mode", type: T_STRING};
	outputs = array_append(outputs, out_name);
	outputs = array_append(outputs, out_mode);

	ProcedureCtx *ctx = ProcCtxNew("dbms.procedures",
	  0,
	  outputs,
	  Proc_ProceduresStep,
	  Proc_ProceduresInvoke,
	  Proc_ProceduresFree,
	  privateData,
	  true);
	return ctx;
}