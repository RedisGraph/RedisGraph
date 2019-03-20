/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "fulltext_query.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../../deps/RediSearch/redisearch_api.h"

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

ProcedureResult fulltextQueryNodeInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) < 2) return PROCEDURE_ERR;

    QueryNodeContext *pdata = rm_malloc(sizeof(QueryNodeContext));
    pdata->output = array_new(SIValue, 4);
    pdata->g = GraphContext_GetFromTLS()->g;
    pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
    pdata->output = array_append(pdata->output, SI_Node(&pdata->n));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
    pdata->output = array_append(pdata->output, SI_DoubleVal(0.0));
    pdata->idx = NULL;
    pdata->iter = NULL;
    ctx->privateData = pdata;

    // See if there's a full-text index for given label.
    char* err = NULL;
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

SIValue* fulltextQueryNodeStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);
    
    QueryNodeContext *pdata = (QueryNodeContext*)ctx->privateData;
    if(!pdata->iter || !pdata->idx) return NULL;

    /* Try to get a result out of the iterator.
     * NULL is returned if iterator id depleted. */
    size_t len = 0;
    NodeID *id = (NodeID*)RediSearch_ResultsIteratorNext(pdata->iter, pdata->idx, &len);
    
    // Depleted.
    if(!id) return NULL;

    // Get Node.
    Node *n = &pdata->n;
    Graph_GetNode(pdata->g, *id, n);

    pdata->output[1] = SI_Node(n);
    pdata->output[3] = SI_DoubleVal(0); // Currently RediSearch does not returns score.
    return pdata->output;
}

ProcedureResult fulltextQueryNodeFree(ProcedureCtx *ctx) {
    // Clean up.
    if(ctx->privateData) {
        QueryNodeContext *pdata = ctx->privateData;
        array_free(pdata->output);
        rm_free(ctx->privateData);
    }
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextQueryNodeGen() {
    void *privateData = NULL;
    char **output = array_new(char*, 2);
    output = array_append(output, "node");
    output = array_append(output, "score");
    ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.queryNodes",
                                    2,
                                    output,
                                    fulltextQueryNodeStep,
                                    fulltextQueryNodeInvoke,
                                    fulltextQueryNodeFree,
                                    privateData);
    return ctx;
}
