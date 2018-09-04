/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_conditional_traverse.h"

// Updates query graph edge.
OpResult _CondTraverse_SetEdge(CondTraverse *op) {
    // Consumed edges connecting current source and destination nodes.
    Edge *e;
    if(!Vector_Pop(op->edges, &e)) return OP_DEPLETED;

    *op->algebraic_expression->edge = e;
    return OP_OK;
}

void _extractColumn(CondTraverse *op) {
    NodeID src_id = (*op->algebraic_expression->src_node)->id;

    // Create hypersparse vector.
    // TODO: Find a quickerway to clear out a vector.
    GrB_Vector v;
    GrB_Vector_new(&v, GrB_BOOL, Graph_NodeCount(op->graph));
    GrB_Vector_setElement_BOOL(v, true, src_id);

    // Append vector to algebraic expression, as the right most operand.
    AlgebraicExpression_AppendTerm(op->algebraic_expression, (GrB_Matrix)v, false, false);

    // Evaluate expression, result is a vector.
    if(op->algebraic_results) AlgebraicExpressionResult_Free(op->algebraic_results);
    op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);    
    op->M = op->algebraic_results->m;
    // Remove vector operand.
    AlgebraicExpression_RemoveTerm(op->algebraic_expression, op->algebraic_expression->operand_count-1, NULL);

    if(op->iter == NULL) op->iter = TuplesIter_new(op->M);
    else TuplesIter_reuse(op->iter, op->M);

    op->iter = TuplesIter_iterate_column(op->iter, 0);
    GrB_Vector_free(&v);
}

OpBase* NewCondTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->graph = g;
    traverse->algebraic_expression = algebraic_expression;
    traverse->algebraic_results = NULL;
    traverse->iter = NULL;
    traverse->edges = NULL;
    
    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Conditional Traverse";
    traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
    traverse->op.consume = CondTraverseConsume;
    traverse->op.reset = CondTraverseReset;
    traverse->op.free = CondTraverseFree;
    traverse->op.modifies = NewVector(char*, 1);

    char *modified = NULL;    
    modified = QueryGraph_GetNodeAlias(qg, *traverse->algebraic_expression->dest_node);
    Vector_Push(traverse->op.modifies, modified);

    if(algebraic_expression->edge) {
        modified = QueryGraph_GetEdgeAlias(qg, *traverse->algebraic_expression->edge);
        Vector_Push(traverse->op.modifies, modified);
        traverse->edges = NewVector(Edge*, 32);
        traverse->edgeRelationType = Edge_GetRelationID(*algebraic_expression->edge);
    }

    return (OpBase*)traverse;
}

/* CondTraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, QueryGraph* graph) {
    CondTraverse *op = (CondTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* Not initialized. */
    if(op->iter == NULL) {
        if(child->consume(child, graph) == OP_DEPLETED) return OP_DEPLETED;
        /* Pick a column. */
        _extractColumn(op);
    }

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->algebraic_expression->edge) {
        if(_CondTraverse_SetEdge(op) == OP_OK) return OP_OK;
    }
    
    NodeID dest_id;
    while(TuplesIter_next(op->iter, &dest_id, NULL) == TuplesIter_DEPLETED) {
        OpResult res = child->consume(child, graph);
        if(res != OP_OK) return res;
        _extractColumn(op);
    }

    /* Get node from current column. */    
    NodeID src_id = (*op->algebraic_expression->src_node)->id;
    Node *dest_node = Graph_GetNode(op->graph, dest_id);
    *op->algebraic_results->dest_node = dest_node;

    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        Graph_GetEdgesConnectingNodes(op->graph, src_id, dest_id, op->edgeRelationType, op->edges);
        return _CondTraverse_SetEdge(op);
    }

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
    
    if(op->edges)
        Vector_Free(op->edges);

    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
}
