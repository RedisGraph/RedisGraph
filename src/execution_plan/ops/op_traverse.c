/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "op_traverse.h"
#include "../../util/arr.h"

// Updates query graph edge.
OpResult _Traverse_SetEdge(Traverse *op, Record *r) {
    // Consumed edges connecting current source and destination nodes.
    uint32_t edgeCount = array_len(op->edges);
    if(!edgeCount) return OP_DEPLETED;

    Edge *e = op->edges + (edgeCount-1);
    op->algebraic_expression->edge->entity = e->entity;
    op->algebraic_expression->edge->srcNodeID= e->srcNodeID;
    op->algebraic_expression->edge->destNodeID= e->destNodeID;
    
    array_pop(op->edges);
    return OP_OK;
}

OpBase* NewTraverseOp(Graph *g, AlgebraicExpression *ae) {
    Traverse *traverse = calloc(1, sizeof(Traverse));
    traverse->graph = g;
    traverse->algebraic_expression = ae;
    traverse->algebraic_results = NULL;
    traverse->edges = NULL;

    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Traverse";
    traverse->op.type = OPType_TRAVERSE;
    traverse->op.consume = TraverseConsume;
    traverse->op.reset = TraverseReset;
    traverse->op.free = TraverseFree;
    traverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = ae->src_node->alias;
    Vector_Push(traverse->op.modifies, modified);
    modified = ae->dest_node->alias;
    Vector_Push(traverse->op.modifies, modified);
    
    if(ae->edge != NULL) {
        modified = ae->edge->alias;
        Vector_Push(traverse->op.modifies, modified);
        traverse->edges = array_new(Edge, Graph_RelationTypeCount(g));
        traverse->edgeRelationType = Edge_GetRelationID(ae->edge);
    }

    return (OpBase*)traverse;
}

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult TraverseConsume(OpBase *opBase, Record *r) {
    Traverse *op = (Traverse*)opBase;
    GrB_Index src_id;
    GrB_Index dest_id;

    if(op->algebraic_results == NULL) {
        op->algebraic_results = AlgebraicExpression_Execute(op->algebraic_expression);
        op->it = TuplesIter_new(op->algebraic_results->m);

        Node *srcNode = op->algebraic_results->src_node;
        Node *destNode = op->algebraic_results->dest_node;
        Record_AddEntry(r, op->algebraic_results->src_node->alias, SI_PtrVal(srcNode));
        Record_AddEntry(r, op->algebraic_results->dest_node->alias, SI_PtrVal(destNode));

        if(op->algebraic_expression->edge != NULL) {
            char *edgeAlias = op->algebraic_expression->edge->alias;
            Record_AddEntry(r, edgeAlias, SI_PtrVal(op->algebraic_expression->edge));
        }
    }

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->algebraic_expression->edge) {
        if(_Traverse_SetEdge(op, r) == OP_OK) return OP_OK;
    }

    if (TuplesIter_next(op->it, &dest_id, &src_id) == TuplesIter_DEPLETED) return OP_DEPLETED;

    Node *srcNode = op->algebraic_results->src_node;
    Node *destNode = op->algebraic_results->dest_node;
    Graph_GetNode(op->graph, src_id, srcNode);
    Graph_GetNode(op->graph, dest_id, destNode);

    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        Graph_GetEdgesConnectingNodes(op->graph,
                                      ENTITY_GET_ID(srcNode),
                                      ENTITY_GET_ID(destNode),
                                      op->edgeRelationType,
                                      &op->edges);
        return _Traverse_SetEdge(op, r);
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
    if(op->it) TuplesIter_free(op->it);
    if(op->edges) array_free(op->edges);
    if(op->algebraic_results) AlgebraicExpressionResult_Free(op->algebraic_results);
}
