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

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.queryNodes(label, query)

typedef struct {
    Node n;
    Graph *g;
    bool stop;
    SIValue *output;
} QueryNodeContext;

ProcedureResult fulltextQueryNodeInvoke(ProcedureCtx *ctx, char **args) {
    QueryNodeContext *pdata = rm_malloc(sizeof(QueryNodeContext));
    pdata->output = array_new(SIValue, 4);
    pdata->stop = false;
    pdata->g = GraphContext_GetFromTLS()->g;
    pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
    pdata->output = array_append(pdata->output, SI_Node(&pdata->n));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
    pdata->output = array_append(pdata->output, SI_DoubleVal(0.0));
    ctx->privateData = pdata;
    return PROCEDURE_OK;
}

SIValue* fulltextQueryNodeStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);

    QueryNodeContext *pdata = (QueryNodeContext*)ctx->privateData;

    // Temporary!
    if(pdata->stop) return NULL;
    pdata->stop = true;

    Node *n = &pdata->n;
    Graph_GetNode(pdata->g, 0, n);

    pdata->output[1] = SI_Node(n);
    pdata->output[3] = SI_DoubleVal(12.34);
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
