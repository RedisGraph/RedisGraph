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

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g) {
    assert(ae && minHops <= maxHops && g && ae->operand_count == 1);
    AST *ast = AST_GetFromLTS();

    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->ae = ae;
    condVarLenTraverse->relationID = Edge_GetRelationID(ae->edge);
    condVarLenTraverse->srcNodeIdx = AST_GetAliasID(ast, ae->src_node->alias);
    condVarLenTraverse->destNodeIdx = AST_GetAliasID(ast, ae->dest_node->alias);
    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->allPathsCtx = NULL;
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
    modified = ae->dest_node->alias;
    Vector_Push(condVarLenTraverse->op.modifies, modified);

    return (OpBase*)condVarLenTraverse;
}

OpResult CondVarLenTraverseConsume(OpBase *opBase, Record r) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];
    OpResult res;

    /* Not initialized. */
    if(!op->allPathsCtx) {
        res = child->consume(child, r);
        if(res != OP_OK) return res;
        Node *srcNode = Record_GetNode(r, op->srcNodeIdx);
        op->allPathsCtx = AllPathsCtx_New(srcNode, op->g, op->relationID, op->traverseDir, op->minHops, op->maxHops);
    }

    Path p = NULL;
    while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
        res = child->consume(child, r);
        if(res != OP_OK) return res;

        Node *srcNode = Record_GetNode(r, op->srcNodeIdx);

        AllPathsCtx_Free(op->allPathsCtx);
        op->allPathsCtx = AllPathsCtx_New(srcNode, op->g, op->relationID, op->traverseDir, op->minHops, op->maxHops);
    }

    // For the timebeing we only care for the last node in path
    Node n = Path_pop(p);
    Path_free(p);

    Record_AddNode(r, op->destNodeIdx, n);
    return OP_OK;
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    AllPathsCtx_Free(op->allPathsCtx);
    op->allPathsCtx = NULL;
    return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    if(op->allPathsCtx) AllPathsCtx_Free(op->allPathsCtx);
    AlgebraicExpression_Free(op->ae);
}
