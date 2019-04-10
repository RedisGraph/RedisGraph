/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))

// Checks to see if operation index is within its bounds.
static inline bool _outOfBounds(OpNodeByIdSeek *op) {
    /* Because currentId starts at minimum and only increases
     * we only care about top bound. */
    if(op->currentId > op->maxId) return true;
    if(op->currentId == op->maxId && !op->maxExclusive) return true;
    return false;
}

OpBase* NewOpNodeByIdSeekOp
(
    const AST *ast,
    unsigned int nodeRecIdx,
    NodeID minId,
    NodeID maxId,
    bool minExclusive,
    bool maxExclusive
)
{
    // Can't include unspecified bound.
    assert(!(minId == ID_RANGE_UNBOUND && minExclusive));
    assert(!(maxId == ID_RANGE_UNBOUND && maxExclusive));

    OpNodeByIdSeek *op_nodeByIdSeek = malloc(sizeof(OpNodeByIdSeek));
    op_nodeByIdSeek->g = GraphContext_GetFromTLS()->g;
    
    op_nodeByIdSeek->minExclusive = minExclusive;
    op_nodeByIdSeek->maxExclusive = maxExclusive;

    // The smallest possible entity ID is 0.
    op_nodeByIdSeek->minId = MAX(0, minId);
    // The largest possible entity ID is the same as Graph_RequiredMatrixDim.
    op_nodeByIdSeek->maxId = MIN(Graph_RequiredMatrixDim(op_nodeByIdSeek->g), maxId);

    op_nodeByIdSeek->currentId = op_nodeByIdSeek->minId;
    if(!minExclusive) op_nodeByIdSeek->currentId++;

    op_nodeByIdSeek->nodeRecIdx = nodeRecIdx;
    op_nodeByIdSeek->recLength = AST_AliasCount(ast);

    OpBase_Init(&op_nodeByIdSeek->op);
    op_nodeByIdSeek->op.name = "NodeByIdSeek";
    op_nodeByIdSeek->op.type = OPType_NODE_BY_ID_SEEK;
    op_nodeByIdSeek->op.consume = OpNodeByIdSeekConsume;
    op_nodeByIdSeek->op.reset = OpNodeByIdSeekReset;
    op_nodeByIdSeek->op.free = OpNodeByIdSeekFree;

    return (OpBase*)op_nodeByIdSeek;
}

Record OpNodeByIdSeekConsume(OpBase *opBase) {
    OpNodeByIdSeek *op = (OpNodeByIdSeek*)opBase;
    Node n;
    n.entity = NULL;

    /* As long as we're within range bounds
     * and we've yet to get a node. */
    while(!_outOfBounds(op)) {
        if(Graph_GetNode(op->g, op->currentId, &n)) break;
        op->currentId++;
    }

    // Advance id for next consume call.
    op->currentId++;

    // Did we managed to get an entity?
    if(!n.entity) return NULL;

    Record r = Record_New(op->recLength);
    Record_AddNode(r, op->nodeRecIdx, n);
    return r;
}

OpResult OpNodeByIdSeekReset(OpBase *ctx) {
    OpNodeByIdSeek *op = (OpNodeByIdSeek*)ctx;
    op->currentId = op->minId;
    if(!op->minExclusive) op->currentId++;
    return OP_OK;
}

void OpNodeByIdSeekFree(OpBase *ctx) {

}
