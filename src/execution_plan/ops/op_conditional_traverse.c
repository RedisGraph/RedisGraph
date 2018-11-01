/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_conditional_traverse.h"
#include "../../GraphBLASExt/GxB_Delete.h"

// Updates query graph edge.
OpResult _CondTraverse_SetEdge(CondTraverse *op, Record *r) {
    // Consumed edges connecting current source and destination nodes.
    Edge *e;
    if(!Vector_Pop(op->edges, &e)) return OP_DEPLETED;

    char *alias = op->algebraic_expression->edge->alias;
    Record_AddEntry(r, alias, SI_PtrVal(e));

    return OP_OK;
}

void _extractColumn(CondTraverse *op, const Record r) {
    Node *n = Record_GetNode(r, op->algebraic_expression->src_node->alias);
    NodeID srcId = n->id;

    GrB_Matrix_setElement_BOOL(op->F, true, srcId, 0);

    // Append matrix to algebraic expression, as the right most operand.
    AlgebraicExpression_AppendTerm(op->algebraic_expression, op->F, false, false);

    // Evaluate expression.
    if(op->algebraic_results) AlgebraicExpressionResult_Free(op->algebraic_results);
    op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);
    op->M = op->algebraic_results->m;
    // Remove operand.
    AlgebraicExpression_RemoveTerm(op->algebraic_expression, op->algebraic_expression->operand_count-1, NULL);

    if(op->iter == NULL) op->iter = TuplesIter_new(op->M);
    else TuplesIter_reuse(op->iter, op->M);

    // Clear filter matrix.
    GxB_Matrix_Delete(op->F, srcId, 0);
}

OpBase* NewCondTraverseOp(Graph *g, AlgebraicExpression *algebraic_expression) {
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
    modified = traverse->algebraic_expression->dest_node->alias;
    Vector_Push(traverse->op.modifies, modified);

    if(algebraic_expression->edge) {
        modified = traverse->algebraic_expression->edge->alias;
        Vector_Push(traverse->op.modifies, modified);
        traverse->edges = NewVector(Edge*, 32);
        traverse->edgeRelationType = Edge_GetRelationID(algebraic_expression->edge);
    }

    return (OpBase*)traverse;
}

/* CondTraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, Record *r) {
    CondTraverse *op = (CondTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* Not initialized. */
    if(op->iter == NULL) {
        if(child->consume(child, r) == OP_DEPLETED) return OP_DEPLETED;
        /* Pick a column. */
        GrB_Matrix_new(&op->F, GrB_BOOL, Graph_NodeCount(op->graph), 1);
        _extractColumn(op, *r);
    }

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->algebraic_expression->edge) {
        if(_CondTraverse_SetEdge(op, r) == OP_OK) return OP_OK;
    }
    
    NodeID dest_id;
    while(TuplesIter_next(op->iter, &dest_id, NULL) == TuplesIter_DEPLETED) {
        OpResult res = child->consume(child, r);
        if(res != OP_OK) return res;
        _extractColumn(op, *r);
    }

    /* Get node from current column. */
    Node *destNode = Graph_GetNode(op->graph, dest_id);
    char *destNodeAlias = op->algebraic_results->dest_node->alias;
    Record_AddEntry(r, destNodeAlias, SI_PtrVal(destNode));

    // TODO If edge were set here, the failing fixed-length test would be resolved.
    // The changes I've thought to make to introduce that, however, have caused other issues.
    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        Node *srcNode = Record_GetNode(*r, op->algebraic_expression->src_node->alias);
        Graph_GetEdgesConnectingNodes(op->graph, srcNode->id, destNode->id, op->edgeRelationType, op->edges);
        return _CondTraverse_SetEdge(op, r);
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
    if(op->iter) TuplesIter_free(op->iter);
    if(op->F) GrB_Matrix_free(&op->F);
    if(op->edges) Vector_Free(op->edges);
    if(op->algebraic_results) AlgebraicExpressionResult_Free(op->algebraic_results);
}
