/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "../../parser/ast.h"
#include "../../util/arr.h"

/* Expand into checks to see if two resolved nodes are connected
 * MATCH (a)-[]->(b), (a)-[:e]->(b) 
 * in the above we might figure out that indeed a is connected somehow to b
 * (a)-[]->(b) 
 * we still need to make sure a is connected via an e edge to b
 * as a and b are already resolved to just need to see if they are 
 * connected via an e edge. */

OpBase* NewExpandIntoOp(Node *a, Node *b, Edge *e) {
    OpExpandInto *expandInto = malloc(sizeof(OpExpandInto));
    AST *ast = AST_GetFromTLS();
    expandInto->srcRecIdx = AST_GetAliasID(ast, a->alias);
    expandInto->destRecIdx = AST_GetAliasID(ast, b->alias);
    expandInto->gc = GraphContext_GetFromTLS();
    expandInto->e = e;
    expandInto->edges = array_new(Edge*, 0);

    // Set our Op operations
    OpBase_Init(&expandInto->op);
    expandInto->op.name = "Expand Into";
    expandInto->op.type = OPType_EXPAND_INTO;
    expandInto->op.consume = OpExpandIntoConsume;
    expandInto->op.reset = OpExpandIntoReset;
    expandInto->op.free = OpExpandIntoFree;
    expandInto->op.modifies = NewVector(char*, 0);
    return (OpBase*)expandInto;
}

Record OpExpandIntoConsume(OpBase *opBase) {
    OpExpandInto *op = (OpExpandInto*)opBase;
    OpBase *child = op->op.children[0];
    Record r = NULL;

    // Clear edges array, see if a is connected to b via e.
    array_clear(op->edges);
    while(array_len(op->edges) == 0) {
        // Try to get data, return NULL if depleted.
        r = child->consume(child);
        if(r == NULL) return NULL;

        // Get node a and b from record.
        Node *a = Record_GetNode(r, op->srcRecIdx);
        Node *b = Record_GetNode(r, op->destRecIdx);
        assert(a && b);

        Graph_GetEdgesConnectingNodes(
            op->gc->g,
            ENTITY_GET_ID(a),
            ENTITY_GET_ID(b),
            Edge_GetRelationID(op->e),
            op->edges
        );
    }

    return r;
}

OpResult OpExpandIntoReset(OpBase *ctx) {
    return OP_OK;
}

void OpExpandIntoFree(OpBase *ctx) {
    OpExpandInto *op = (OpExpandInto*)ctx;
    array_free(op->edges);
}
