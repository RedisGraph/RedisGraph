/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.createNodeIndex(label, fields...)
// CALL db.idx.fulltext.createNodeIndex('book', 'title', 'authors')
ProcedureResult Proc_FulltextCreateNodeIdxInvoke(ProcedureCtx *ctx, const char **args) {
	if(array_len(args) < 2) return PROCEDURE_ERR;

	// Create full-text index.
	const char *label = args[0];
	uint fields_count = array_len(args) - 1;
	const char **fields = args + 1; // Skip index name.

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Index *idx = GraphContext_GetIndex(gc, label, NULL, IDX_FULLTEXT);

	// Index doesn't exists, create.
	if(idx == NULL) {
		GraphContext_AddIndex(&idx, gc, label, fields[0], IDX_FULLTEXT);
	}

	// Introduce fields to index.
	for(int i = 0; i < fields_count; i++) {
		const char *field = fields[i];
		// It's OK to add existing field.
		Index_AddField(idx, field);
	}

	// Build index.
	Index_Construct(idx);

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
	ProcedureOutput **output = array_new(ProcedureOutput *, 0);
	ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.createNodeIndex",
								   PROCEDURE_VARIABLE_ARG_COUNT,
								   output,
								   Proc_FulltextCreateNodeIdxStep,
								   Proc_FulltextCreateNodeIdxInvoke,
								   Proc_FulltextCreateNodeIdxFree,
								   privateData);

	return ctx;
}
