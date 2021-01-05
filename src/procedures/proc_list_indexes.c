/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_list_indexes.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../datatypes/array.h"
#include "RG.h"

typedef struct {
	SIValue *out;               // outputs
	int schema_id;              // current schema ID
	IndexType type;             // current index type to retrieve
	GraphContext *gc;           // graph context
	SIValue *yield_type;        // yield index type
	SIValue *yield_label;       // yield index label
	SIValue *yield_properties;  // yield index properties
} IndexesContext;

// CALL db.indexes()
ProcedureResult Proc_IndexesInvoke(ProcedureCtx *ctx, const SIValue *args,
								   const char **yield) {

	GraphContext *gc = QueryCtx_GetGraphCtx();

	// TODO: introduce invoke validation, similar to arithmetic expressions
	// expecting no arguments
	uint arg_count = array_len((SIValue *)args);
	if(arg_count != 0) return PROCEDURE_ERR;

	IndexesContext *pdata = rm_malloc(sizeof(IndexesContext));
	pdata->gc               = gc;
	pdata->out              = array_new(SIValue, 8);
	pdata->schema_id        = GraphContext_SchemaCount(gc, SCHEMA_NODE) - 1;
	pdata->type             = IDX_EXACT_MATCH;
	pdata->yield_type       = NULL;
	pdata->yield_label      = NULL;
	pdata->yield_properties = NULL;

	ASSERT(yield);
	uint yield_count = array_len(yield);
	for(uint i = 0; i < yield_count; i++) {
		if(strcasecmp("type", yield[i]) == 0) {
			pdata->out = array_append(pdata->out, SI_ConstStringVal("type"));
			pdata->out = array_append(pdata->out, SI_NullVal());
			pdata->yield_type = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("label", yield[i]) == 0) {
			pdata->out = array_append(pdata->out, SI_ConstStringVal("label"));
			pdata->out = array_append(pdata->out, SI_NullVal());
			pdata->yield_label = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("properties", yield[i]) == 0) {
			pdata->out = array_append(pdata->out, SI_ConstStringVal("properties"));
			pdata->out = array_append(pdata->out, SI_NullVal());
			pdata->yield_properties = pdata->out + (i * 2 + 1);
			continue;
		}
	}

	ctx->privateData = pdata;

	return PROCEDURE_OK;
}

static bool _Proc_IndexesEmitIndex(IndexesContext *ctx, Schema *s, IndexType idx_type) {
	Index *idx = Schema_GetIndex(s, NULL, idx_type);
	if(idx == NULL) return false;

	if(ctx->yield_type != NULL) {
		if(idx_type == IDX_EXACT_MATCH) {
			*ctx->yield_type = SI_ConstStringVal("exact-match");
		} else {
			*ctx->yield_type = SI_ConstStringVal("full-text");
		}
	}

	if(ctx->yield_label) {
		*ctx->yield_label = SI_ConstStringVal((char *)Index_GetLabel(idx));
	}

	if(ctx->yield_properties) {
		uint fields_count        = Index_FieldsCount(idx);
		*ctx->yield_properties   = SI_Array(fields_count);
		const char **fields      = Index_GetFields(idx);

		for(uint i = 0; i < fields_count; i++) {
			SIArray_Append(ctx->yield_properties, SI_ConstStringVal((char *)fields[i]));
		}
	}

	return true;
}

SIValue *Proc_IndexesStep(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData != NULL);

	IndexesContext *pdata = ctx->privateData;

	// Loop over all schemas from last to first.
	while(pdata->schema_id >= 0) {
		Schema *s = GraphContext_GetSchemaByID(pdata->gc, pdata->schema_id, SCHEMA_NODE);
		if(!Schema_HasIndices(s)) {
			// No indexes found, continue to the next schema.
			pdata->schema_id--;
			continue;
		}

		// Populate index data if one is found.
		bool found = _Proc_IndexesEmitIndex(pdata, s, pdata->type);

		if(pdata->type == IDX_FULLTEXT) {
			// All indexes retrieved; update schema_id.
			pdata->schema_id--;
			pdata->type = IDX_EXACT_MATCH;
		} else {
			// The next iteration will check the same schema for a full-text index.
			pdata->type = IDX_FULLTEXT;
		}

		if(found) return pdata->out;
	}

	return NULL;
}

ProcedureResult Proc_IndexesFree(ProcedureCtx *ctx) {
	// clean up
	if(ctx->privateData) {
		IndexesContext *pdata = ctx->privateData;
		array_free(pdata->out);
		rm_free(pdata);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_IndexesCtx() {
	void *privateData = NULL;
	ProcedureOutput output;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 3);

	// index type (exact-match / fulltext)
	output  = (ProcedureOutput) {
		.name = "type", .type = T_STRING
	};
	outputs = array_append(outputs, output);

	// indexed label
	output  = (ProcedureOutput) {
		.name = "label", .type = T_STRING
	};
	outputs = array_append(outputs, output);

	// indexed properties
	output  = (ProcedureOutput) {
		.name = "properties", .type = T_ARRAY
	};
	outputs = array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("db.indexes",
								   0,
								   outputs,
								   Proc_IndexesStep,
								   Proc_IndexesInvoke,
								   Proc_IndexesFree,
								   privateData,
								   true);
	return ctx;
}

