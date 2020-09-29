/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_labels.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

// CALL db.labels()

typedef struct {
	uint schema_id;     // Current schema ID.
	GraphContext *gc;   // Graph context.
	SIValue *output;    // Output label.
} LabelsContext;

ProcedureResult Proc_LabelsInvoke(ProcedureCtx *ctx,
		const SIValue *args, const char **yield) {
	if(array_len((SIValue *)args) != 0) return PROCEDURE_ERR;

	LabelsContext *pdata = rm_malloc(sizeof(LabelsContext));
	pdata->schema_id = 0;
	pdata->gc = QueryCtx_GetGraphCtx();
	pdata->output = array_new(SIValue, 2);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("label"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

SIValue *Proc_LabelsStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	LabelsContext *pdata = (LabelsContext *)ctx->privateData;

	// Depleted?
	if(pdata->schema_id >= GraphContext_SchemaCount(pdata->gc, SCHEMA_NODE))
		return NULL;

	// Get schema label.
	Schema *s = GraphContext_GetSchemaByID(pdata->gc, pdata->schema_id++, SCHEMA_NODE);
	char *label = (char *)Schema_GetName(s);
	pdata->output[1] = SI_ConstStringVal(label);
	return pdata->output;
}

ProcedureResult Proc_LabelsFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		LabelsContext *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_LabelsCtx() {
	void *privateData = NULL;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 1);
	ProcedureOutput output = {name: "label", type: T_STRING};
	outputs = array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("db.labels",
								   0,
								   outputs,
								   Proc_LabelsStep,
								   Proc_LabelsInvoke,
								   Proc_LabelsFree,
								   privateData,
								   true);
	return ctx;
}

