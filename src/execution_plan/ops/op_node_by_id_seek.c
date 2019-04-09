/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"

OpBase* NewOpNodeByIdSeekOp(const AST *ast, Node *n, NodeID nodeId) {
    OpNodeByIdSeek *op_nodeByIdSeek = malloc(sizeof(OpNodeByIdSeek));    
    op_nodeByIdSeek->id = nodeId;
    op_nodeByIdSeek->g = GraphContext_GetFromTLS()->g;
    op_nodeByIdSeek->nodeRecIdx = AST_GetAliasID(ast, n->alias);
    op_nodeByIdSeek->recLength = AST_AliasCount(ast);
    op_nodeByIdSeek->entityRetrieved = false;

    OpBase_Init(&op_nodeByIdSeek->op);
    op_nodeByIdSeek->op.name = "NodeByIdSeek";
    op_nodeByIdSeek->op.type = OPType_NODE_BY_ID_SEEK;
    op_nodeByIdSeek->op.consume = OpNodeByIdSeekConsume;
    op_nodeByIdSeek->op.reset = OpNodeByIdSeekReset;
    op_nodeByIdSeek->op.free = OpNodeByIdSeekFree;

    return (OpBase*)op_nodeByIdSeek;
}

Record OpNodeByIdSeekConsume(OpBase *opBase) {
    Node n;
    OpNodeByIdSeek *op = (OpNodeByIdSeek*)opBase;
    
    /* Try to get entity by ID 
     * Incase entity doesn't exists return depleted 
     * otherwise place entity within recoed and
     * mark that entity was retrieved. */

    // Can only retrieve entity once.
    if(op->entityRetrieved) return NULL;

    // See if entity with ID exists in the graph.
    if(!Graph_GetNode(op->g, op->id, &n)) return NULL;
    
    // Managed to get entity, mark and set record.
    op->entityRetrieved = true;
    Record r = Record_New(op->recLength);
    Record_AddNode(r, op->nodeRecIdx, n);
    return r;
}

OpResult OpNodeByIdSeekReset(OpBase *ctx) {
    OpNodeByIdSeek *op = (OpNodeByIdSeek*)ctx;
    op->entityRetrieved = false;
    return OP_OK;
}

void OpNodeByIdSeekFree(OpBase *ctx) {

}
