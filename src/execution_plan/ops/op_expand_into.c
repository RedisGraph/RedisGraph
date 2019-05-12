/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "../../util/arr.h"

// Updates query graph edge.
static int _SetEdge(OpExpandInto *op, Record r) {
    // Consumed edges connecting current source and destination nodes.
    if(!array_len(op->edges)) return 0;

    Edge *e = op->edges + (array_len(op->edges)-1);
    Record_AddEdge(r, op->edgeRecIdx, *e);
    array_pop(op->edges);
    return 1;
}

/* Expand into checks to see if two resolved nodes are connected
 * MATCH (a)-[]->(b), (a)-[:e]->(b) 
 * in the above we might figure out that indeed a is connected somehow to b
 * (a)-[]->(b) 
 * we still need to make sure a is connected via an e edge to b
 * as a and b are already resolved we just need to see if they are 
 * connected via an e edge. */

OpBase* NewExpandIntoOp(AlgebraicExpression *exp, uint srcRecIdx, uint destRecIdx, uint edgeRecIdx) {
    OpExpandInto *expandInto = malloc(sizeof(OpExpandInto));
    expandInto->r = NULL;
    expandInto->exp = exp;
    expandInto->srcRecIdx = srcRecIdx;
    expandInto->destRecIdx = destRecIdx;
    expandInto->edgeRecIdx = edgeRecIdx;
    expandInto->edges = NULL;
    expandInto->g = GraphContext_GetFromTLS()->g;
    expandInto->relation = (exp->edge) ? Edge_GetRelationID(exp->edge) : GRAPH_NO_RELATION;

    // Set our Op operations
    OpBase_Init(&expandInto->op);
    expandInto->op.name = "Expand Into";
    expandInto->op.type = OPType_EXPAND_INTO;
    expandInto->op.consume = OpExpandIntoConsume;
    expandInto->op.reset = OpExpandIntoReset;
    expandInto->op.free = OpExpandIntoFree;

    if(exp->edge) {
        expandInto->op.modifies = NewVector(char*, 1);
        Vector_Push(expandInto->op.modifies, exp->edge->alias);
        expandInto->edges = array_new(Edge, 32);
    }

    return (OpBase*)expandInto;
}

Record OpExpandIntoConsume(OpBase *opBase) {
    Node *a;
    Node *b;
    NodeID aID;
    NodeID bID;
    OpExpandInto *op = (OpExpandInto*)opBase;
    OpBase *child = op->op.children[0];

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->exp->edge) {
        if(_SetEdge(op, op->r)) {
            return Record_Clone(op->r);
        } else {
            // No more edges to consume, free previous record.
            if(op->r) Record_Clone(op->r);
        }
    }

    // As long as we don't have a connection between src and dest.
    while(true) {
        // Try to get data, return NULL if depleted.
        op->r = child->consume(child);
        if(op->r == NULL) return NULL;

        // Get node a and b from record.
        a = Record_GetNode(op->r, op->srcRecIdx);
        b = Record_GetNode(op->r, op->destRecIdx);
        assert(a && b);
        aID = ENTITY_GET_ID(a);
        bID = ENTITY_GET_ID(b);

        if(Graph_EdgeExists(op->g, aID, bID, op->relation)) break;
    }

    if(op->exp->edge) {
        Graph_GetEdgesConnectingNodes(op->g, aID, bID, op->relation, &op->edges);
        _SetEdge(op, op->r);
        return Record_Clone(op->r);
    } else {
        return op->r;
    }
}

OpResult OpExpandIntoReset(OpBase *ctx) {
    OpExpandInto *op = (OpExpandInto*)ctx;
    if(op->edges) array_clear(op->edges);
    return OP_OK;
}

void OpExpandIntoFree(OpBase *ctx) {
    OpExpandInto *op = (OpExpandInto*)ctx;
    if(op->r) Record_Clone(op->r);
    if(op->edges) array_free(op->edges);
    if(op->exp) AlgebraicExpression_Free(op->exp);
}
