/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_create_index.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../index/index.h"
#include "../datatypes/datatypes.h"
#include "../errors.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// validate index configuration map
// label is requierd and of type string
// configuration can't change if index exists 
// stopwords is array of strings
// language is string
static ProcedureResult _validateLabel(SIValue label) {
	SIValue si_value;
	if(Map_Get(label, SI_ConstStringVal("label"), &si_value)) {
		if(Map_KeyCount(label) > 1) {
			GraphContext *gc = QueryCtx_GetGraphCtx();
			Schema *s = GraphContext_GetSchema(gc, si_value.stringval, SCHEMA_NODE);
			if(s) {
				Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
				if(idx) {
					ErrorCtx_SetError("Index already exists configuration can't be changed");
					return PROCEDURE_ERR;
				}
			}
		}
	} else {
		ErrorCtx_SetError("Label is missing");
		return PROCEDURE_ERR;
	}

	if(Map_Get(label, SI_ConstStringVal("stopwords"), &si_value)) {
		if(SI_TYPE(si_value) == T_ARRAY) {
			int stopwords_count = SIArray_Length(si_value);
			for (int i = 0; i < stopwords_count; i++) {
				SIValue stopword = SIArray_Get(si_value, i);
				if(SI_TYPE(stopword) != T_STRING) {
					ErrorCtx_SetError("Stopword must be a string");
					return PROCEDURE_ERR;
				}
			}
		} else {
			ErrorCtx_SetError("Stopwords must be array");
			return PROCEDURE_ERR;
		}
	}
	if(Map_Get(label, SI_ConstStringVal("language"), &si_value)) {
		if(SI_TYPE(si_value) != T_STRING) {
			ErrorCtx_SetError("Language must be string");
			return PROCEDURE_ERR;
		}
	}

	return PROCEDURE_OK;
}

// CALL db.idx.fulltext.createNodeIndex(label, fields...)
// CALL db.idx.fulltext.createNodeIndex('book', 'title', 'authors')
ProcedureResult Proc_FulltextCreateNodeIdxInvoke(ProcedureCtx *ctx,
		const SIValue *args, const char **yield) {
	uint arg_count = array_len((SIValue *)args);
	if(arg_count < 2) {
		ErrorCtx_SetError("Minimum number of arguments is 2");
		return PROCEDURE_ERR;
	}

	// validation, all arguments should be of type string
	if(!(SI_TYPE(args[0]) & (T_STRING | T_MAP))) {
		ErrorCtx_SetError("Label argument can be string or map");
		return PROCEDURE_ERR;
	}
	if(SI_TYPE(args[0]) == T_MAP && _validateLabel(args[0]) == PROCEDURE_ERR) {
		return PROCEDURE_ERR;
	}

	for(uint i = 1; i < arg_count; i++) {
		if(!(SI_TYPE(args[i]) & T_STRING)) {
			ErrorCtx_SetError("Field arguments must be string");
			return PROCEDURE_ERR;
		}
	}

	// create full-text index
	SIValue si_value;
	int res               = INDEX_FAIL;
	Index *idx            = NULL;
	GraphContext *gc      = QueryCtx_GetGraphCtx();
	uint fields_count     = arg_count - 1;
	const char *label     = NULL;
	const SIValue *fields = args + 1; // skip index name

	if(SI_TYPE(args[0]) == T_STRING) {
		label = args[0].stringval;
	} else if(SI_TYPE(args[0]) == T_MAP) {
		Map_Get(args[0], SI_ConstStringVal("label"), &si_value);
		label = si_value.stringval;
	}

	// introduce fields to index
	for(int i = 0; i < fields_count; i++) {
		SIValue field = fields[i];
		if(GraphContext_AddIndex(&idx, gc, label, field.stringval, IDX_FULLTEXT) == INDEX_OK) {
			res = INDEX_OK;
		}
	}

	if(SI_TYPE(args[0]) == T_MAP) {
		if(Map_Get(args[0], SI_ConstStringVal("stopwords"), &si_value)) {
			int stopwords_count = SIArray_Length(si_value);
			idx->stopwords = array_new(char*, stopwords_count);
			for (int i = 0; i < stopwords_count; i++) {
				SIValue stopword = SIArray_Get(si_value, i);
				array_append(idx->stopwords, rm_strdup(stopword.stringval));
			}
		}
		if(Map_Get(args[0], SI_ConstStringVal("language"), &si_value)) {
			idx->language = rm_strdup(si_value.stringval);
		}
	}

	// build index
	if(res == INDEX_OK) Index_Construct(idx);

	return PROCEDURE_OK;
}

SIValue *Proc_FulltextCreateNodeIdxStep(ProcedureCtx *ctx) {
	return NULL;
}

ProcedureResult Proc_FulltextCreateNodeIdxFree(ProcedureCtx *ctx) {
	// Clean up.
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_FulltextCreateNodeIdxGen() {
	void *privateData = NULL;
	ProcedureOutput *output = array_new(ProcedureOutput, 0);
	ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.createNodeIndex",
								   PROCEDURE_VARIABLE_ARG_COUNT,
								   output,
								   Proc_FulltextCreateNodeIdxStep,
								   Proc_FulltextCreateNodeIdxInvoke,
								   Proc_FulltextCreateNodeIdxFree,
								   privateData,
								   false);

	return ctx;
}

