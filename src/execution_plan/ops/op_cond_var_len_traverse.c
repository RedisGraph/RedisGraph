/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, GraphContext *gc) {
    assert(ae && minHops <= maxHops && gc && ae->operand_count == 1);
    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->gc = gc;
    condVarLenTraverse->relationID = Edge_GetRelationID(ae->edge);
    condVarLenTraverse->srcNodeAlias = ae->src_node->alias;
    condVarLenTraverse->destNodeAlias = ae->dest_node->alias;
    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->pathsCount = 0;
    condVarLenTraverse->pathsCap = 32;
    condVarLenTraverse->paths = malloc(sizeof(Path) * condVarLenTraverse->pathsCap);
    condVarLenTraverse->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING : GRAPH_EDGE_DIR_OUTGOING;

    // Set our Op operations
    OpBase_Init(&condVarLenTraverse->op);
    condVarLenTraverse->op.name = "Conditional Variable Length Traverse";
    condVarLenTraverse->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
    condVarLenTraverse->op.consume = CondVarLenTraverseConsume;
    condVarLenTraverse->op.reset = CondVarLenTraverseReset;
    condVarLenTraverse->op.free = CondVarLenTraverseFree;
    condVarLenTraverse->op.modifies = NewVector(char*, 1);

    const char *modified = NULL;
    modified = condVarLenTraverse->destNodeAlias;
    Vector_Push(condVarLenTraverse->op.modifies, modified);

    return (OpBase*)condVarLenTraverse;
}

OpResult CondVarLenTraverseConsume(OpBase *opBase, Record *r) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];

    OpResult res;

    /* Not initialized. */
    while(op->pathsCount == 0) {
        res = child->consume(child, r);
        if(res != OP_OK) return res;
        Node *srcNode = Record_GetNode(*r, op->srcNodeAlias);
        op->pathsCount = AllPaths(op->g, op->relationID, srcNode->id, op->traverseDir, op->minHops, op->maxHops, &op->pathsCap, &op->paths);
    }

    Path p = op->paths[--op->pathsCount];
    Edge *e = Path_pop(p);
    Node *neighbor;
    if(op->traverseDir == GRAPH_EDGE_DIR_OUTGOING) neighbor = Edge_GetDestNode(e);
    else neighbor = Edge_GetSrcNode(e);

    Path_free(p);
    Record_AddEntry(r, op->destNodeAlias, SI_PtrVal(neighbor));

    return OP_OK;
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    for(int i = 0; i < op->pathsCount; i++) Path_free(op->paths[i]);
    op->pathsCount = 0;
    // TODO: I think Reset should propegate to child nodes.
    return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    for(int i = 0; i < op->pathsCount; i++) Path_free(op->paths[i]);
    free(op->paths);
}
