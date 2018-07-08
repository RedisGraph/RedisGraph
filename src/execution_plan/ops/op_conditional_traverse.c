#include "op_conditional_traverse.h"

OpBase* NewCondTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression) {
    return (OpBase*)NewCondTraverse(g, qg, algebraic_expression);
}

CondTraverse* NewCondTraverse(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->graph = g;
    traverse->algebraic_expression = algebraic_expression;
    traverse->algebraic_results = NULL;

    // Set our Op operations
    traverse->op.name = "Conditional Traverse";
    traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
    traverse->op.consume = CondTraverseConsume;
    traverse->op.reset = CondTraverseReset;
    traverse->op.free = CondTraverseFree;
    traverse->state = CondTraverseUninitialized;
    traverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->src_node);
    Vector_Push(traverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->dest_node);
    Vector_Push(traverse->op.modifies, modified);

    return traverse;
}

void extractColumn(CondTraverse *op) {
    GrB_Index column_idx = (*op->algebraic_results->dest_node)->id;
    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, op->M);

    GrB_Col_extract(op->V, NULL, NULL, op->M, GrB_ALL, nrows, column_idx, NULL);
    TuplesIter_reuse(op->iter, (GrB_Matrix)op->V);
}

/* CondTraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, QueryGraph* graph) {
    CondTraverse *op = (CondTraverse*)opBase;
    
    /* Not initialized. */
    if(op->state == CondTraverseUninitialized) {
        op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);
        op->M = op->algebraic_results->m;

        GrB_Index nrows;
        GrB_Matrix_nrows(&nrows, op->M);
        GrB_Vector_new(&(op->V), GrB_BOOL, nrows);
        op->iter = TuplesIter_new((GrB_Matrix) op->V);
        return OP_REFRESH;
    }

    /* Pick a column. */
    if(op->state == CondTraverseResetted) {
        op->state = CondTraverseConsuming;
        extractColumn(op);
    }

    /* Get node from current column. */
    GrB_Index src_id;
    if(TuplesIter_next(op->iter, &src_id, NULL) == TuplesIter_DEPLETED) return OP_REFRESH;

    Node *src_node = Graph_GetNode(op->graph, src_id);
    src_node->id = src_id;
    *op->algebraic_results->src_node = src_node;

    return OP_OK;
}

OpResult CondTraverseReset(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    TuplesIter_reset(op->iter);
    op->state = CondTraverseResetted;
    return OP_OK;
}

/* Frees CondTraverse */
void CondTraverseFree(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    TuplesIter_free(op->iter);
    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
    free(op);
}
