/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_create_index.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../index/index.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/datatypes.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// validate index configuration map
// [required] label <string>
// [optional] stopwords <string[]>
// [optional] language <string>
// configuration can't change if index exists 
static ProcedureResult _validateIndexConfigMap(SIValue config) {
	SIValue sw;
	SIValue lang;
	SIValue label;

	bool multi_config = Map_KeyCount(config) > 1;
	bool label_exists = Map_Get(config, SI_ConstStringVal("label"), &label);
	bool stopword_exists = Map_Get(config, SI_ConstStringVal("stopwords"), &sw);
	bool lang_exists = Map_Get(config, SI_ConstStringVal("language"), &lang);

	if(!label_exists) {
		ErrorCtx_SetError("Label is missing");
		return PROCEDURE_ERR;
	}

	if(multi_config) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Schema *s = GraphContext_GetSchema(gc, label.stringval, SCHEMA_NODE);
		if(s && Schema_GetIndex(s, NULL, IDX_FULLTEXT)) {
			ErrorCtx_SetError("Index already exists configuration can't be changed");
			return PROCEDURE_ERR;
		}
	}

	if(stopword_exists) {
		if(SI_TYPE(sw) == T_ARRAY) {
			uint stopwords_count = SIArray_Length(sw);
			for (uint i = 0; i < stopwords_count; i++) {
				SIValue stopword = SIArray_Get(sw, i);
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

	if(lang_exists) {
		if(SI_TYPE(lang) != T_STRING) {
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

	// label argument should be of type string or map
	if(!(SI_TYPE(args[0]) & (T_STRING | T_MAP))) {
		ErrorCtx_SetError("Label argument can be string or map");
		return PROCEDURE_ERR;
	}
	// validation, fields arguments should be of type string
	if(SI_TYPE(args[0]) == T_MAP &&
			_validateIndexConfigMap(args[0]) == PROCEDURE_ERR) {
		return PROCEDURE_ERR;
	}

	for(uint i = 1; i < arg_count; i++) {
		if(!(SI_TYPE(args[i]) & T_STRING)) {
			ErrorCtx_SetError("Field arguments must be string");
			return PROCEDURE_ERR;
		}
	}

	// create full-text index
	SIValue value;
	int res               = INDEX_FAIL;
	Index *idx            = NULL;
	GraphContext *gc      = QueryCtx_GetGraphCtx();
	uint fields_count     = arg_count - 1;
	const char *label     = NULL;
	const SIValue *fields = args + 1; // skip index name

	if(SI_TYPE(args[0]) == T_STRING) {
		label = args[0].stringval;
	} else if(SI_TYPE(args[0]) == T_MAP) {
		Map_Get(args[0], SI_ConstStringVal("label"), &value);
		label = value.stringval;
	}

	// introduce fields to index
	for(uint i = 0; i < fields_count; i++) {
		const char *field = fields[i].stringval;
		res = GraphContext_AddIndex(&idx, gc, label, field, IDX_FULLTEXT);
	}

	// TODO: similar to the validation function above
	// introduce an arguments extract function which given argv
	// will populate label, stopwords and language
	assert(false);

	if(SI_TYPE(args[0]) == T_MAP) {
		if(Map_Get(args[0], SI_ConstStringVal("stopwords"), &value)) {
			uint stopwords_count = SIArray_Length(value);
			idx->stopwords = array_new(char*, stopwords_count);
			for (uint i = 0; i < stopwords_count; i++) {
				SIValue stopword = SIArray_Get(value, i);
				array_append(idx->stopwords, rm_strdup(stopword.stringval));
			}
		}
		if(Map_Get(args[0], SI_ConstStringVal("language"), &value)) {
			idx->language = rm_strdup(value.stringval);
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

