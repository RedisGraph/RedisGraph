/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_query.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../redisearch_api.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.queryNodes(label, query)

typedef struct {
	Node n;
	Graph *g;
	SIValue *output;
	RSIndex *idx;
	RSResultsIterator *iter;
} QueryNodeContext;

ProcedureResult Proc_FulltextQueryNodeInvoke(ProcedureCtx *ctx, char **args) {
	if(array_len(args) < 2) return PROCEDURE_ERR;

	QueryNodeContext *pdata = rm_malloc(sizeof(QueryNodeContext));
	pdata->output = array_new(SIValue, 2);
	pdata->g = GraphContext_GetFromTLS()->g;
	pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
	pdata->output = array_append(pdata->output, SI_Node(&pdata->n));
	// pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
	// pdata->output = array_append(pdata->output, SI_DoubleVal(0.0));
	pdata->idx = NULL;
	pdata->iter = NULL;
	ctx->privateData = pdata;

	// See if there's a full-text index for given label.
	char *err = NULL;
	const char *label = args[0];
	const char *query = args[1];
	GraphContext *gc = GraphContext_GetFromTLS();

	// Get full-text index from schema.
	Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	if(s == NULL) return PROCEDURE_OK;
	pdata->idx = Schema_GetFullTextIndex(s);
	if(!pdata->idx) return PROCEDURE_OK;

	// Execute query
	pdata->iter = RediSearch_IterateQuery(pdata->idx, query, strlen(query), &err);
	// TODO: report error!
	if(err) pdata->iter = NULL;
	return PROCEDURE_OK;
}

SIValue *Proc_FulltextQueryNodeStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	QueryNodeContext *pdata = (QueryNodeContext *)ctx->privateData;
	if(!pdata->iter || !pdata->idx) return NULL;

	/* Try to get a result out of the iterator.
	 * NULL is returned if iterator id depleted. */
	size_t len = 0;
	NodeID *id = (NodeID *)RediSearch_ResultsIteratorNext(pdata->iter, pdata->idx,
	             &len);

	// Depleted.
	if(!id) return NULL;

	// Get Node.
	Node *n = &pdata->n;
	Graph_GetNode(pdata->g, *id, n);

	pdata->output[1] = SI_Node(n);
	return pdata->output;
}

ProcedureResult Proc_FulltextQueryNodeFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		QueryNodeContext *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_FulltextQueryNodeGen() {
	void *privateData = NULL;
	ProcedureOutput **output = array_new(ProcedureOutput *, 1);
	ProcedureOutput *out_node = rm_malloc(sizeof(ProcedureOutput));
	out_node->name = "node";
	out_node->type = T_NODE;

	output = array_append(output, out_node);
	ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.queryNodes",
	                               2,
	                               output,
	                               Proc_FulltextQueryNodeStep,
	                               Proc_FulltextQueryNodeInvoke,
	                               Proc_FulltextQueryNodeFree,
	                               privateData);
	return ctx;
}
