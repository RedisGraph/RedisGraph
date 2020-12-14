/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../util/arr.h"
#include "rax.h"
#include "../RG.h"
#include "procedure.h"
#include "proc_procedures.h"

extern rax *__procedures;

// CALL dbms.procedures()

typedef struct {
	SIValue *output;      // Array with a maximum of 4 entries: ["name", name, "mode", mode].
	raxIterator iter;     // Procedures iterator.
} ProcProceduresPrivateData;

ProcedureResult Proc_ProceduresInvoke(ProcedureCtx *ctx,
  		const SIValue *args, const char **yield) {
	if(array_len((SIValue *)args) != 0) return PROCEDURE_ERR;

	ProcProceduresPrivateData *pdata = rm_malloc(sizeof(ProcProceduresPrivateData));

	// Initialize an iterator to the rax that contains all procedures
	rax *procedures = __procedures;
	raxStart(&pdata->iter, procedures);
	raxSeek(&pdata->iter, "^", NULL, 0);
	pdata->output = array_new(SIValue, 4);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("name"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("mode"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

// Promote the rax iterator to the next procedure and return its name and mode.
SIValue *Proc_ProceduresStep(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData);

	ProcProceduresPrivateData *pdata = (ProcProceduresPrivateData *)ctx->privateData;
	// Depleted?
	if(!raxNext(&pdata->iter)) return NULL;

	// Returns the current procedure's name and mode.
	ProcGenerator gen = pdata->iter.data;
	ProcedureCtx *curr_proc_ctx = gen();
	pdata->output[1] = SI_ConstStringVal((char *)Procedure_GetName(curr_proc_ctx));
	pdata->output[3] = Procedure_IsReadOnly(curr_proc_ctx) ?
	  SI_ConstStringVal("READ") : SI_ConstStringVal("WRITE");
	Proc_Free(curr_proc_ctx);
	return pdata->output;
}

ProcedureResult Proc_ProceduresFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		ProcProceduresPrivateData *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_ProceduresCtx() {
	void *privateData = NULL;

	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput out_name = {.name = "name", .type = T_STRING};
	ProcedureOutput out_mode = {.name = "mode", .type = T_STRING};
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
