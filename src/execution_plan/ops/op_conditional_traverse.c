/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_conditional_traverse.h"

OpBase* NewCondTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->graph = g;
    traverse->algebraic_expression = algebraic_expression;
    traverse->algebraic_results = NULL;
    traverse->iter = NULL;

    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Conditional Traverse";
    traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
    traverse->op.consume = CondTraverseConsume;
    traverse->op.reset = CondTraverseReset;
    traverse->op.free = CondTraverseFree;
    traverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->src_node);
    Vector_Push(traverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->dest_node);
    Vector_Push(traverse->op.modifies, modified);

    return (OpBase*)traverse;
}

void extractColumn(CondTraverse *op) {
    GrB_Index column_idx = (*op->algebraic_results->dest_node)->id;
    op->iter = TuplesIter_iterate_column(op->iter, column_idx);
}

/* CondTraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, QueryGraph* graph) {
    CondTraverse *op = (CondTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* Not initialized. */
    if(op->iter == NULL) {
        op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);
        op->M = op->algebraic_results->m;
        op->iter = TuplesIter_new(op->M);

        if(child->consume(child, graph) == OP_DEPLETED) return OP_DEPLETED;

        /* Pick a column. */
        extractColumn(op);
    }

    /* Get node from current column. */
    GrB_Index src_id;
    while(TuplesIter_next(op->iter, &src_id, NULL) == TuplesIter_DEPLETED) {
        OpResult res = child->consume(child, graph);
        if(res != OP_OK) return res;
        extractColumn(op);
    }

    Node *src_node = Graph_GetNode(op->graph, src_id);
    src_node->id = src_id;
    *op->algebraic_results->src_node = src_node;
    return OP_OK;
}

OpResult CondTraverseReset(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    TuplesIter_reset(op->iter);
    return OP_OK;
}

/* Frees CondTraverse */
void CondTraverseFree(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    TuplesIter_free(op->iter);
    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
}
