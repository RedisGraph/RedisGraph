/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_traverse.h"
#include <assert.h>

// Updates query graph edge.
OpResult _Traverse_SetEdge(Traverse *op) {
    // Consumed edges connecting current source and destination nodes.
    Edge *e = EdgeIterator_Next(op->edgeIter);
    if(e == NULL) return OP_DEPLETED;
    
    *op->algebraic_expression->edge = e;
    return OP_OK;
}

OpBase* NewTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *ae) {
    Traverse *traverse = calloc(1, sizeof(Traverse));
    traverse->graph = g;
    traverse->algebraic_expression = ae;
    traverse->algebraic_results = NULL;
    traverse->edgeIter = NULL;

    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Traverse";
    traverse->op.type = OPType_TRAVERSE;
    traverse->op.consume = TraverseConsume;
    traverse->op.reset = TraverseReset;
    traverse->op.free = TraverseFree;
    traverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *ae->src_node);
    Vector_Push(traverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *ae->dest_node);
    Vector_Push(traverse->op.modifies, modified);
    
    if(ae->edge != NULL) {
        traverse->edgeIter = EdgeIterator_New();
        traverse->edgeRelationType = Edge_GetRelationID(*ae->edge);
        modified = QueryGraph_GetEdgeAlias(qg, *ae->edge);
        Vector_Push(traverse->op.modifies, modified);
    }

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

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->algebraic_expression->edge) {
        if(_Traverse_SetEdge(op) == OP_OK) return OP_OK;
    }

    if (TuplesIter_next(op->it, &dest_id, &src_id) == TuplesIter_DEPLETED) return OP_DEPLETED;

    Node *srcNode = Graph_GetNode(op->graph, src_id);
    Node *destNode = Graph_GetNode(op->graph, dest_id);
    *op->algebraic_results->src_node = srcNode;
    *op->algebraic_results->dest_node = destNode;

    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        EdgeIterator_Reuse(op->edgeIter);
        Graph_GetEdgesConnectingNodes(op->graph, src_id, dest_id, op->edgeRelationType, op->edgeIter);
        return _Traverse_SetEdge(op);
    }

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

    if(op->edgeIter)
        EdgeIterator_Free(op->edgeIter);

    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
}
