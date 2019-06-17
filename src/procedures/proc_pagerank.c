/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./proc_pagerank.h"
#include "../util/arr.h"
#include "../graph/graph.h"
#include "../graph/graphcontext.h"
#include "../../deps/LAGraph/Include/LAGraph.h"

typedef struct {
    LAGraph_PageRank *ranking;      // Sorted ranking.
    GrB_Index i;                    // Result index to yield.
    GrB_Index n;                    // Number of nodes ranked.
    Graph *g;                       // Graph.
    Node node;                      // Current node.
    SIValue *output;                // Output node + rank.
} PageRankContext;

// CALL algo.pageRank('Page', 'LINKS')
ProcedureResult Proc_PageRankInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) != 2) return PROCEDURE_ERR;

    const char *label = args[0];
    const char *relation = args[1];
    assert(label && relation);

    Graph *g;
    Schema *s;
    int label_id;
    int relation_id;
    GraphContext *gc;

    GrB_Info res;
    GrB_Matrix L;
    GrB_Matrix R;
    GrB_Matrix A;
    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Index nvals;
    GrB_Descriptor desc;

    int iters = 0;
    double tol = 1e-5;
    int iterations = 20;
    LAGraph_PageRank *Phandle;

    gc = GraphContext_GetFromTLS();
    g = gc->g;

    s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
    if(!s) return PROCEDURE_ERR;

    label_id = s->id;
    L = Graph_GetLabelMatrix(g, label_id);
    
    s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
    if(!s) return PROCEDURE_ERR;
    
    relation_id = s->id;
    R = Graph_GetRelationMatrix(g, relation_id);
    
    nrows = Graph_RequiredMatrixDim(g);
    ncols = Graph_RequiredMatrixDim(g);

    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

    GrB_Matrix_new(&A, GrB_BOOL, nrows, ncols);

    // Using R as a mask, as we're filtering R.
    GrB_mxm(A, R, GrB_NULL, Rg_structured_bool, R, L, GrB_NULL);
    // Using A as a mask, as we're filtering A.
    GrB_mxm(A, A, GrB_NULL, Rg_structured_bool, L, A, desc);

    // TODO: Remove once CSR is merged.
    GrB_transpose(A, GrB_NULL, GrB_NULL, A, GrB_NULL);

    // Number of nodes to be ranked.
    GrB_Matrix_nvals(&nvals, A);

    res = LAGraph_pagerank(
        &Phandle,       // output: array of LAGraph_PageRank structs
        A,              // binary input graph, not modified
        iterations,     // max number of iterations
        tol,            // stop when norm (r-rnew,2) < tol
        &iters          // number of iterations taken
    );

    // Clean up.
    GrB_free(&A);
    GrB_free(&desc);

    // Setup private data.
    PageRankContext *pdata = rm_malloc(sizeof(PageRankContext));
    pdata->i = 0;
    pdata->g = g;
    pdata->n = nvals;
    pdata->ranking = Phandle;
    pdata->output = array_new(SIValue, 4);

    pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.
    pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.
    ctx->privateData = pdata;

    return PROCEDURE_OK;
}

SIValue* Proc_PageRankStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);

    PageRankContext *pdata = (PageRankContext*)ctx->privateData;

    // Depleted?
    if(pdata->i >= pdata->n) return NULL;

    // Get schema label.
    LAGraph_PageRank rank = pdata->ranking[pdata->i++];
    NodeID node_id = (NodeID)rank.page;

    Graph_GetNode(pdata->g, node_id, &pdata->node);

    // Setup output.
    pdata->output[1] = SI_Node(&pdata->node);
    pdata->output[3] = SI_DoubleVal(rank.pagerank);
    return pdata->output;
}

ProcedureResult Proc_PageRankFree(ProcedureCtx *ctx) {
    // Clean up.
    if(ctx->privateData) {
        PageRankContext *pdata = ctx->privateData;
        free(pdata->ranking);
        array_free(pdata->output);
        rm_free(ctx->privateData);
    }

    return PROCEDURE_OK;
}

ProcedureCtx* Proc_PageRankCtx() {
    void *privateData = NULL;
    ProcedureOutput **outputs = array_new(ProcedureOutput*, 2);
    
    ProcedureOutput *node_output = rm_malloc(sizeof(ProcedureOutput));
    node_output->name = "node";
    node_output->type = T_NODE;
    outputs = array_append(outputs, node_output);

    ProcedureOutput *score_output = rm_malloc(sizeof(ProcedureOutput));
    score_output->name = "score";
    score_output->type = T_DOUBLE;
    outputs = array_append(outputs, score_output);

    ProcedureCtx *ctx = ProcCtxNew("algo.pageRank",
                                    2,
                                    outputs,
                                    Proc_PageRankStep,
                                    Proc_PageRankInvoke,
                                    Proc_PageRankFree,
                                    privateData);
    return ctx;
}

