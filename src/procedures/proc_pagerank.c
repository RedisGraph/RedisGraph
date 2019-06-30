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
    GrB_Index *mapping;             // Maps between translated node ID to original node ID.
    GrB_Index i;                    // Result index to yield.
    GrB_Index n;                    // Number of nodes ranked.
    Graph *g;                       // Graph.
    Node node;                      // Current node.
    SIValue *output;                // Output node + rank.
} PageRankContext;

/* Compact matrix, creates a new matrix out of R (a relation matrix) 
 * where only row i and column i remain, where L[i,i] = 1.
 * this will prodice a matrix C where #rows and #cols in C <=  #rows and #cols in R 
 * in addition this has the nice side-effect similar to L*R*L e.g.
 * focusing only on entries in R representing nodes labeled as L. */
static GrB_Matrix _compact_matrix(GrB_Matrix L, GrB_Matrix R, GrB_Index **mapping) {
    GrB_Index nvals = 0;
    GrB_Matrix_nvals(&nvals, L);
    
    GrB_Matrix C;

    // Because L is diagonal, number of nodes equals number of entries.
    GrB_Index nrows = nvals;
    GrB_Index ncols = nvals;
    GrB_Matrix_new(&C, GrB_BOOL, nrows, ncols);

    // Extract row indicies.
    GrB_Index ni = nvals;
    GrB_Index *I = malloc(sizeof(GrB_Index) * ni);

    // Caller will need to free I.
    *mapping = I;

    GrB_Info res = GrB_Matrix_extractTuples_BOOL           // [I,J,X] = find (A)
    (
        I,          // array for returning row indices of tuples
        GrB_NULL,   // array for returning col indices of tuples
        GrB_NULL,   // array for returning values of tuples
        &nvals,     // I,J,X size on input; # tuples on output
        L           // matrix to extract tuples from
    );
    assert(res == GrB_SUCCESS);

    /* For every node N of type L
     * retrieve all possible connections from n to every other node 
     * of type L. */
    res = GrB_extract   // C<Mask> = accum (C, A(I,J))
    (
        C,          // input/output matrix for results
        GrB_NULL,   // optional mask for C, unused if NULL
        GrB_NULL,   // optional accum for Z=accum(C,T)
        R,          // first input:  matrix A
        I,          // row indices
        ni,         // number of row indices
        I,          // column indices
        ni,         // number of column indices
        GrB_NULL    // descriptor for C, Mask, and A
    );
    assert(res == GrB_SUCCESS);

    return C;
}

// CALL algo.pageRank('Page', 'LINKS')
ProcedureResult Proc_PageRankInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) != 2) return PROCEDURE_ERR;

    const char *label = args[0];
    const char *relation = args[1];
    assert(label && relation);    
   
    GraphContext *gc = GraphContext_GetFromTLS();
    Graph *g = gc->g;

    Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
    if(!s) return PROCEDURE_ERR;

    int label_id = s->id;
    GrB_Matrix L = Graph_GetLabelMatrix(g, label_id);

    s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
    if(!s) return PROCEDURE_ERR;

    int relation_id = s->id;
    GrB_Matrix R = Graph_GetRelationMatrix(g, relation_id);

    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, R);

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, L);

    bool freeC;
    GrB_Matrix C;
    GrB_Index *mapping = NULL;

    if(nrows == nvals) {
        /* R doesn't contains information other than
         * connections between nodes labeled as L. */
        C = R;
        freeC = false;
    } else {
        /* Create a compact representation of L*R*L
        * this will avoid computing pagerank for nodes
        * not labeled as L. */
        C = _compact_matrix(L, R, &mapping);
        freeC = true;
        // Number of nodes to be ranked.
        GrB_Matrix_nrows(&nrows, C);
    }

    int iters = 0;
    double tol = 1e-5;
    int iterations = 20;
    LAGraph_PageRank *Phandle = NULL;

    GrB_Info res = LAGraph_pagerank(
        &Phandle,       // output: array of LAGraph_PageRank structs
        C,              // binary input graph, not modified
        iterations,     // max number of iterations
        tol,            // stop when norm (r-rnew,2) < tol
        &iters          // number of iterations taken
    );    
    assert(res == GrB_SUCCESS);

    if(freeC) GrB_free(&C);

    // Setup private data.
    PageRankContext *pdata = rm_malloc(sizeof(PageRankContext));
    pdata->i = 0;
    pdata->g = g;
    pdata->n = nrows;
    pdata->ranking = Phandle;
    pdata->mapping = mapping;
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

    LAGraph_PageRank rank = pdata->ranking[pdata->i++];
    NodeID node_id = (NodeID)rank.page;

    // Required to performing mapping.
    if (pdata->mapping) node_id = pdata->mapping[node_id];

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
        if(pdata->mapping) free(pdata->mapping);
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
