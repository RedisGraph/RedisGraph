/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_traverse.h"
#include <assert.h>

// Get edges connecting source node to destination node.
void _Traverse_GetEdges(Traverse *op, Node *srcNode, Node *destNode) {
    Edge *e = *op->algebraic_expression->edge;
    size_t edgeCap = op->edgeCap;

    /* Get edgeCap edges, if we're giving exactly edgeCap
     * it is possible that there are additional edges,
     * and so we'll reallocate to accommodate additional edges. */ 
    while(true) {
        Graph_GetEdgesConnectingNodes(op->graph, srcNode->id, destNode->id,
                                  Edge_GetRelationID(e), op->edges, &edgeCap);
        
        // Indication for additional edges.
        if (edgeCap == op->edgeCap) {
            op->edgeCap *= 2;
            op->edges = realloc(op->edges, sizeof(Edge*) * op->edgeCap);
            edgeCap = op->edgeCap;
        } else {
            break;
        }
    }

    op->edgeCount = edgeCap;
    op->edgeIdx = 0;
}

// Updates query graph edge.
OpResult _Traverse_SetEdge(Traverse *op) {
    // Consumed edges connecting current source and destination nodes.
    if(op->edgeIdx >= op->edgeCount) return OP_DEPLETED;

    Edge *e = op->edges[op->edgeIdx++];
    *op->algebraic_expression->edge = e;

    return OP_OK;
}

OpBase* NewTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *ae) {
    Traverse *traverse = calloc(1, sizeof(Traverse));
    traverse->graph = g;
    traverse->algebraic_expression = ae;
    traverse->algebraic_results = NULL;
    traverse->edgeIdx = 0;
    traverse->edgeCap = 0;
    traverse->edgeCount = 0;
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
    modified = QueryGraph_GetNodeAlias(qg, *ae->src_node);
    Vector_Push(traverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *ae->dest_node);
    Vector_Push(traverse->op.modifies, modified);
    
    if(ae->edge != NULL) {
        traverse->edgeCap = 1024;
        traverse->edges = malloc(sizeof(Edge*) * traverse->edgeCap);

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
        _Traverse_GetEdges(op, srcNode, destNode);
        return _Traverse_SetEdge(op);
    }

    return OP_OK;
}

OpResult TraverseReset(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_reset(op->it);
    op->edgeIdx = op->edgeCount = 0;
    return OP_OK;
}

/* Frees Traverse */
void TraverseFree(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_free(op->it);
    if(op->algebraic_results)
        AlgebraicExpressionResult_Free(op->algebraic_results);
    if(op->edges)
        free(op->edges);
}
