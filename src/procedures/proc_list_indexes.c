/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "proc_list_indexes.h"
#include "RG.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../datatypes/map.h"
#include "../datatypes/array.h"

typedef struct {
	SIValue *out;               // outputs
	int node_schema_id;         // current node schema ID
	int edge_schema_id;         // current edge schema ID
	IndexType type;             // current index type to retrieve
	GraphContext *gc;           // graph context
	SIValue *yield_type;        // yield index type
	SIValue *yield_label;       // yield index label
	SIValue *yield_properties;  // yield index properties
	SIValue *yield_language;    // yield index language
	SIValue *yield_stopwords;   // yield index stopwords
	SIValue *yield_entity_type; // yield index entity type
	SIValue *yield_status;      // yield index status
	SIValue *yield_info;        // yield info
} IndexesContext;

static void _process_yield
(
	IndexesContext *ctx,
	const char **yield
) {
	ctx->yield_type        = NULL;
	ctx->yield_info        = NULL;
	ctx->yield_label       = NULL;
	ctx->yield_status      = NULL;
	ctx->yield_language    = NULL;
	ctx->yield_stopwords   = NULL;
	ctx->yield_properties  = NULL;
	ctx->yield_entity_type = NULL;

	int idx = 0;
	for(uint i = 0; i < array_len(yield); i++) {
		if(strcasecmp("type", yield[i]) == 0) {
			ctx->yield_type = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("label", yield[i]) == 0) {
			ctx->yield_label = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("properties", yield[i]) == 0) {
			ctx->yield_properties = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("language", yield[i]) == 0) {
			ctx->yield_language = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("stopwords", yield[i]) == 0) {
			ctx->yield_stopwords = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("entitytype", yield[i]) == 0) {
			ctx->yield_entity_type = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("status", yield[i]) == 0) {
			ctx->yield_status = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("info", yield[i]) == 0) {
			ctx->yield_info = ctx->out + idx;
			idx++;
			continue;
		}
	}
}

// CALL db.indexes()
ProcedureResult Proc_IndexesInvoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {

	ASSERT(ctx   != NULL);
	ASSERT(args  != NULL);
	ASSERT(yield != NULL);

	// TODO: introduce invoke validation, similar to arithmetic expressions
	// expecting no arguments
	uint arg_count = array_len((SIValue *)args);
	if(arg_count != 0) return PROCEDURE_ERR;

	GraphContext *gc = QueryCtx_GetGraphCtx();

	IndexesContext *pdata = rm_malloc(sizeof(IndexesContext));

	pdata->gc             = gc;
	pdata->out            = array_new(SIValue, 8);
	pdata->type           = IDX_EXACT_MATCH;
	pdata->node_schema_id = GraphContext_SchemaCount(gc, SCHEMA_NODE) - 1;
	pdata->edge_schema_id = GraphContext_SchemaCount(gc, SCHEMA_EDGE) - 1;

	_process_yield(pdata, yield);

	ctx->privateData = pdata;

	return PROCEDURE_OK;
}

static bool _EmitIndex
(
	IndexesContext *ctx,
	const Schema *s,
	IndexType type
) {
	Index idx = Schema_GetIndex(s, NULL, type);
	if(idx == NULL) return false;

	//--------------------------------------------------------------------------
	// index entity type
	//--------------------------------------------------------------------------

	if(ctx->yield_entity_type != NULL) {
		if(s->type == SCHEMA_NODE) {
			*ctx->yield_entity_type = SI_ConstStringVal("NODE");
		} else {
			*ctx->yield_entity_type = SI_ConstStringVal("RELATIONSHIP");
		}
	}

	//--------------------------------------------------------------------------
	// index status
	//--------------------------------------------------------------------------

	if(ctx->yield_status != NULL) {
		if(Index_Enabled(idx)) {
			*ctx->yield_status = SI_ConstStringVal("OPERATIONAL");
		} else {
			*ctx->yield_status = SI_ConstStringVal("UNDER CONSTRUCTION");
		}
	}

	//--------------------------------------------------------------------------
	// index type
	//--------------------------------------------------------------------------

	if(ctx->yield_type != NULL) {
		if(type == IDX_EXACT_MATCH) {
			*ctx->yield_type = SI_ConstStringVal("exact-match");
		} else {
			*ctx->yield_type = SI_ConstStringVal("full-text");
		}
	}

	//--------------------------------------------------------------------------
	// index label
	//--------------------------------------------------------------------------

	if(ctx->yield_label) {
		*ctx->yield_label = SI_ConstStringVal((char *)Schema_GetName(s));
	}

	//--------------------------------------------------------------------------
	// index fields
	//--------------------------------------------------------------------------

	if(ctx->yield_properties) {
		uint fields_count        = Index_FieldsCount(idx);
		const IndexField *fields = Index_GetFields(idx);
		*ctx->yield_properties   = SI_Array(fields_count);

		for(uint i = 0; i < fields_count; i++) {
			SIArray_Append(ctx->yield_properties,
						   SI_ConstStringVal((char *)fields[i].name));
		}
	}

	//--------------------------------------------------------------------------
	// index language
	//--------------------------------------------------------------------------

	if(ctx->yield_language) {
		*ctx->yield_language =
			SI_ConstStringVal((char *)Index_GetLanguage(idx));
	}

	//--------------------------------------------------------------------------
	// index stopwords
	//--------------------------------------------------------------------------

	if(ctx->yield_stopwords) {
		size_t stopwords_count;
		char **stopwords = Index_GetStopwords(idx, &stopwords_count);
		if(stopwords) {
			*ctx->yield_stopwords = SI_Array(stopwords_count);
			for(size_t i = 0; i < stopwords_count; i++) {
				SIValue value = SI_ConstStringVal(stopwords[i]);
				SIArray_Append(ctx->yield_stopwords, value);
				rm_free(stopwords[i]);
			}
		} else {
			*ctx->yield_stopwords = SI_Array(0);
		}
		rm_free(stopwords);
	}

	//--------------------------------------------------------------------------
	// index info
	//--------------------------------------------------------------------------

	if(ctx->yield_info) {
		RSIdxInfo info = { .version = RS_INFO_CURRENT_VERSION };

		RediSearch_IndexInfo(Index_RSIndex(idx), &info);
		SIValue map = SI_Map(23);

		Map_Add(&map, SI_ConstStringVal("gcPolicy"),  SI_LongVal(info.gcPolicy));
		Map_Add(&map, SI_ConstStringVal("score"),     SI_DoubleVal(info.score));
		Map_Add(&map, SI_ConstStringVal("lang"),      SI_ConstStringVal(info.lang));

		SIValue fields = SIArray_New(info.numFields);
		for (uint i = 0; i < info.numFields; i++) {
			struct RSIdxField f = info.fields[i];
			SIValue field = SI_Map(6);
			Map_Add(&field, SI_ConstStringVal("path"),             SI_ConstStringVal(f.path));
			Map_Add(&field, SI_ConstStringVal("name"),             SI_ConstStringVal(f.name));
			Map_Add(&field, SI_ConstStringVal("types"),            SI_LongVal(f.types));
			Map_Add(&field, SI_ConstStringVal("options"),          SI_LongVal(f.options));
			Map_Add(&field, SI_ConstStringVal("textWeight"),       SI_DoubleVal(f.textWeight));
			Map_Add(&field, SI_ConstStringVal("tagCaseSensitive"), SI_BoolVal(f.tagCaseSensitive));
			SIArray_Append(&fields, field);
			SIValue_Free(field);
		}
		Map_Add(&map, SI_ConstStringVal("fields"), fields);
		SIValue_Free(fields);

		Map_Add(&map, SI_ConstStringVal("numDocuments"),     SI_LongVal(info.numDocuments));
		Map_Add(&map, SI_ConstStringVal("maxDocId"),         SI_LongVal(info.maxDocId));
		Map_Add(&map, SI_ConstStringVal("docTableSize"),     SI_LongVal(info.docTableSize));
		Map_Add(&map, SI_ConstStringVal("sortablesSize"),    SI_LongVal(info.sortablesSize));
		Map_Add(&map, SI_ConstStringVal("docTrieSize"),      SI_LongVal(info.docTrieSize));
		Map_Add(&map, SI_ConstStringVal("numTerms"),         SI_LongVal(info.numTerms));
		Map_Add(&map, SI_ConstStringVal("numRecords"),       SI_LongVal(info.numRecords));
		Map_Add(&map, SI_ConstStringVal("invertedSize"),     SI_LongVal(info.invertedSize));
		Map_Add(&map, SI_ConstStringVal("invertedCap"),      SI_LongVal(info.invertedCap));
		Map_Add(&map, SI_ConstStringVal("skipIndexesSize"),  SI_LongVal(info.skipIndexesSize));
		Map_Add(&map, SI_ConstStringVal("scoreIndexesSize"), SI_LongVal(info.scoreIndexesSize));
		Map_Add(&map, SI_ConstStringVal("offsetVecsSize"),   SI_LongVal(info.offsetVecsSize));
		Map_Add(&map, SI_ConstStringVal("offsetVecRecords"), SI_LongVal(info.offsetVecRecords));
		Map_Add(&map, SI_ConstStringVal("termsSize"),        SI_LongVal(info.termsSize));
		Map_Add(&map, SI_ConstStringVal("indexingFailures"), SI_LongVal(info.indexingFailures));
		Map_Add(&map, SI_ConstStringVal("totalCollected"),   SI_LongVal(info.totalCollected));
		Map_Add(&map, SI_ConstStringVal("numCycles"),        SI_LongVal(info.numCycles));
		Map_Add(&map, SI_ConstStringVal("totalMSRun"),       SI_LongVal(info.totalMSRun));
		Map_Add(&map, SI_ConstStringVal("lastRunTimeMs"),    SI_LongVal(info.lastRunTimeMs));

		RediSearch_IndexInfoFree(&info);
		*ctx->yield_info = map;
	}

	return true;
}

static SIValue *Schema_Step
(
	int *schema_id,
	SchemaType t,
	IndexesContext *pdata
) {
	Schema *s = NULL;

	// loop over all schemas from last to first
	while(*schema_id >= 0) {
		s = GraphContext_GetSchemaByID(pdata->gc, *schema_id, t);
		if(!Schema_HasIndices(s)) {
			// no indexes found, continue to the next schema
			(*schema_id)--;
			continue;
		}

		// populate index data if one is found
		bool found = _EmitIndex(pdata, s, pdata->type);

		if(pdata->type == IDX_FULLTEXT) {
			// all indexes retrieved; update schema_id, reset schema type
			(*schema_id)--;
			pdata->type = IDX_EXACT_MATCH;
		} else {
			// next iteration will check the same schema for a full-text index
			pdata->type = IDX_FULLTEXT;
		}

		if(found) return pdata->out;
	}

	return NULL;
}

SIValue *Proc_IndexesStep
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);

	SIValue *res;
	IndexesContext *pdata = ctx->privateData;

	res = Schema_Step(&pdata->node_schema_id, SCHEMA_NODE, pdata);
	if(res != NULL) return res;

	return Schema_Step(&pdata->edge_schema_id, SCHEMA_EDGE, pdata);
}

ProcedureResult Proc_IndexesFree
(
	ProcedureCtx *ctx
) {
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
	ProcedureOutput *outputs = array_new(ProcedureOutput, 8);

	// index type (exact-match / fulltext)
	output = (ProcedureOutput) {
		.name = "type", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed label
	output = (ProcedureOutput) {
		.name = "label", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed properties
	output = (ProcedureOutput) {
		.name = "properties", .type = T_ARRAY
	};
	array_append(outputs, output);

	// indexed language
	output = (ProcedureOutput) {
		.name = "language", .type = T_STRING
	};
	array_append(outputs, output);

	// indexed stopwords
	output = (ProcedureOutput) {
		.name = "stopwords", .type = T_ARRAY
	};
	array_append(outputs, output);

	// index entity type (node / relationship)
	output = (ProcedureOutput) {
		.name = "entitytype", .type = T_STRING
	};
	array_append(outputs, output);

	// index status (operational / under construction)
	output = (ProcedureOutput) {
		.name = "status", .type = T_STRING
	};
	array_append(outputs, output);

	// index info
	output = (ProcedureOutput) {
		.name = "info", .type = T_MAP
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

