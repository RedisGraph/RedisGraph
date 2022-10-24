/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_drop_index.h"
#include "../query_ctx.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

//------------------------------------------------------------------------------
// fulltext dropNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.drop(label)
// CALL db.idx.fulltext.drop('books')

ProcedureResult Proc_FulltextDropIndexInvoke(ProcedureCtx *ctx,
		const SIValue *args, const char **yield) {
	if(array_len((SIValue *)args) != 1) return PROCEDURE_ERR;
	if(!(SI_TYPE(args[0]) & T_STRING)) return PROCEDURE_ERR;

	const char *l = args[0].stringval;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int res = GraphContext_DeleteIndex(gc, SCHEMA_NODE, l, NULL, IDX_FULLTEXT);

	if(res != INDEX_OK) {
		ErrorCtx_SetError("ERR Unable to drop index on :%s: no such index.", l);
	}

	return PROCEDURE_OK;
}

SIValue *Proc_FulltextDropIndexStep(ProcedureCtx *ctx) {
	return NULL;
}

ProcedureResult Proc_FulltextDropIndexFree(ProcedureCtx *ctx) {
	// Clean up.
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_FulltextDropIdxGen() {
	void *privateData = NULL;
	ProcedureOutput *output = array_new(ProcedureOutput, 0);
	ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.drop",
								   1,
								   output,
								   Proc_FulltextDropIndexStep,
								   Proc_FulltextDropIndexInvoke,
								   Proc_FulltextDropIndexFree,
								   privateData,
								   false);

	return ctx;
}
