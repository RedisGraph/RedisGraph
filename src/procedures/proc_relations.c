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

// CALL db.relationshipTypes()

typedef struct {
	uint schema_id;     // Current schema ID.
	GraphContext *gc;   // Graph context.
	SIValue *output;    // Output label.
} RelationsContext;

ProcedureResult Proc_RelationsInvoke(ProcedureCtx *ctx, const SIValue *args, const char **yield) {
	if(array_len((SIValue *)args) != 0) return PROCEDURE_ERR;

	RelationsContext *pdata = rm_malloc(sizeof(RelationsContext));
	pdata->schema_id = 0;
	pdata->gc = QueryCtx_GetGraphCtx();
	pdata->output = array_new(SIValue, 2);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("relationshipType"));
	pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

SIValue *Proc_RelationsStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	RelationsContext *pdata = (RelationsContext *)ctx->privateData;

	// Depleted?
	if(pdata->schema_id >= GraphContext_SchemaCount(pdata->gc, SCHEMA_EDGE))
		return NULL;

	// Get schema name.
	Schema *s = GraphContext_GetSchemaByID(pdata->gc, pdata->schema_id++, SCHEMA_EDGE);
	char *name = (char *)Schema_GetName(s);
	pdata->output[1] = SI_ConstStringVal(name);
	return pdata->output;
}

ProcedureResult Proc_RelationsFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		RelationsContext *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_RelationsCtx() {
	void *privateData = NULL;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 1);
	ProcedureOutput output = {name: "relationshipType", type: T_STRING};
	outputs = array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("db.relationshipTypes",
								   0,
								   outputs,
								   Proc_RelationsStep,
								   Proc_RelationsInvoke,
								   Proc_RelationsFree,
								   privateData,
								   true);
	return ctx;
}

