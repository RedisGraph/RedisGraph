/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_list_indexes.h"
#include "RG.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../datatypes/array.h"

typedef struct {
	SIValue *out;               // outputs
	int schema_id;              // current schema ID
	IndexType type;             // current index type to retrieve
	GraphContext *gc;           // graph context
	SIValue *yield_type;        // yield index type
	SIValue *yield_label;       // yield index label
	SIValue *yield_properties;  // yield index properties
	SIValue *yield_language;    // yield index language
	SIValue *yield_stopwords;   // yield index stopwords
} IndexesContext;

// CALL db.indexes()
ProcedureResult Proc_IndexesInvoke(ProcedureCtx *ctx, const SIValue *args,
								   const char **yield) {

	ASSERT(ctx != NULL && args != NULL && yield != NULL);

	// TODO: introduce invoke validation, similar to arithmetic expressions
	// expecting no arguments
	uint arg_count = array_len((SIValue *)args);
	if(arg_count != 0) return PROCEDURE_ERR;

	GraphContext *gc = QueryCtx_GetGraphCtx();

	IndexesContext *pdata   = rm_malloc(sizeof(IndexesContext));
	pdata->gc               = gc;
	pdata->out              = array_new(SIValue, 6);
	pdata->type             = IDX_EXACT_MATCH;
	pdata->schema_id        = GraphContext_SchemaCount(gc, SCHEMA_NODE) - 1;
	pdata->yield_type       = NULL;
	pdata->yield_label      = NULL;
	pdata->yield_properties = NULL;
	pdata->yield_language   = NULL;
	pdata->yield_stopwords  = NULL;

	uint yield_count = array_len(yield);
	for(uint i = 0; i < yield_count; i++) {
		if(strcasecmp("type", yield[i]) == 0) {
			array_append(pdata->out, SI_ConstStringVal("type"));
			array_append(pdata->out, SI_NullVal());
			pdata->yield_type = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("label", yield[i]) == 0) {
			array_append(pdata->out, SI_ConstStringVal("label"));
			array_append(pdata->out, SI_NullVal());
			pdata->yield_label = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("properties", yield[i]) == 0) {
			array_append(pdata->out, SI_ConstStringVal("properties"));
			array_append(pdata->out, SI_NullVal());
			pdata->yield_properties = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("language", yield[i]) == 0) {
			array_append(pdata->out, SI_ConstStringVal("language"));
			array_append(pdata->out, SI_NullVal());
			pdata->yield_language = pdata->out + (i * 2 + 1);
			continue;
		}
		if(strcasecmp("stopwords", yield[i]) == 0) {
			array_append(pdata->out, SI_ConstStringVal("stopwords"));
			array_append(pdata->out, SI_NullVal());
			pdata->yield_stopwords = pdata->out + (i * 2 + 1);
			continue;
		}
	}

	ctx->privateData = pdata;

	return PROCEDURE_OK;
}

static bool _EmitIndex(IndexesContext *ctx, const Schema *s, IndexType type) {
	Index *idx = Schema_GetIndex(s, NULL, type);
	if(idx == NULL) return false;

	if(ctx->yield_type != NULL) {
		if(type == IDX_EXACT_MATCH) {
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
		const char **fields      = Index_GetFields(idx);
		*ctx->yield_properties   = SI_Array(fields_count);

		for(uint i = 0; i < fields_count; i++) {
			SIArray_Append(ctx->yield_properties,
					SI_ConstStringVal((char *)fields[i]));
		}
	}

	if(ctx->yield_language) {
		*ctx->yield_language = 
			SI_ConstStringVal(idx->language ? idx->language : "English");
	}

	if(ctx->yield_stopwords) {
		if(idx->stopwords) {
			const char **stopwords  = (const char **)idx->stopwords;
			uint stopwords_count    = array_len(stopwords);
			*ctx->yield_stopwords   = SI_Array(stopwords_count);
			for (uint i = 0; i < stopwords_count; i++){
				SIArray_Append(ctx->yield_stopwords,
					SI_ConstStringVal((char *)stopwords[i]));
			}
		} else {
			*ctx->yield_stopwords = SI_Array(0);
		}
	}

	return true;
}

SIValue *Proc_IndexesStep(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData != NULL);

	Schema *s = NULL;
	IndexesContext *pdata = ctx->privateData;

	// loop over all schemas from last to first
	while(pdata->schema_id >= 0) {
		s = GraphContext_GetSchemaByID(pdata->gc, pdata->schema_id, SCHEMA_NODE);
		if(!Schema_HasIndices(s)) {
			// no indexes found, continue to the next schema
			pdata->schema_id--;
			continue;
		}

		// populate index data if one is found
		bool found = _EmitIndex(pdata, s, pdata->type);

		if(pdata->type == IDX_FULLTEXT) {
			// all indexes retrieved; update schema_id, reset schema type
			pdata->schema_id--;
			pdata->type = IDX_EXACT_MATCH;
		} else {
			// next iteration will check the same schema for a full-text index
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
	ProcedureOutput *outputs = array_new(ProcedureOutput, 5);

	// index type (exact-match / fulltext)
	output  = (ProcedureOutput) {
		.name = "type", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed label
	output  = (ProcedureOutput) {
		.name = "label", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed properties
	output  = (ProcedureOutput) {
		.name = "properties", .type = T_ARRAY
	};
	array_append(outputs, output);

	// indexed language
	output  = (ProcedureOutput) {
		.name = "language", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed stopwords
	output  = (ProcedureOutput) {
		.name = "stopwords", .type = T_ARRAY
	};
	array_append(outputs, output);

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

