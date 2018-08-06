/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_traverse.h"
#include <assert.h>

OpBase* NewTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *ae) {
    Traverse *traverse = calloc(1, sizeof(Traverse));
    traverse->graph = g;
    traverse->algebraic_expression = ae;
    traverse->algebraic_results = NULL;    

    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Traverse";
    traverse->op.type = OPType_TRAVERSE;
    traverse->op.consume = TraverseConsume;
    traverse->op.reset = TraverseReset;
    traverse->op.free = TraverseFree;
    traverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->src_node);
    Vector_Push(traverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->dest_node);
    Vector_Push(traverse->op.modifies, modified);

    return (OpBase*)traverse;
}

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult TraverseConsume(OpBase *opBase, QueryGraph* graph) {
    Traverse *op = (Traverse*)opBase;
    GrB_Index src_id;
    GrB_Index dest_id;

    if(op->algebraic_results == NULL) {
        op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);
        op->it = TuplesIter_new(op->algebraic_results->m);
    }

    if (TuplesIter_next(op->it, &dest_id, &src_id) == TuplesIter_DEPLETED) return OP_DEPLETED;

    Node *src_node = Graph_GetNode(op->graph, src_id);
    Node *dest_node = Graph_GetNode(op->graph, dest_id);
    src_node->id = src_id;
    dest_node->id = dest_id;

    *op->algebraic_results->src_node = src_node;
    *op->algebraic_results->dest_node = dest_node;

    return OP_OK;
}

OpResult TraverseReset(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_reset(op->it);
    return OP_OK;
}

/* Frees Traverse */
void TraverseFree(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_free(op->it);
    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
}
