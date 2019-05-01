/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../parser/ast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g) {
    assert(ae && minHops <= maxHops && g && ae->operand_count == 1);

    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->ae = ae;
    condVarLenTraverse->edgeRelationTypes = NULL;

    condVarLenTraverse->srcNodeIdx = ae->src_node_idx;
    condVarLenTraverse->destNodeIdx = ae->dest_node_idx;

    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->allPathsCtx = NULL;
    condVarLenTraverse->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING : GRAPH_EDGE_DIR_OUTGOING;
    condVarLenTraverse->r = NULL;

    condVarLenTraverse->edgeRelationTypes = ae->relation_ids;
    condVarLenTraverse->edgeRelationCount = array_len(ae->relation_ids);

    // Set our Op operations
    OpBase_Init(&condVarLenTraverse->op);
    condVarLenTraverse->op.name = "Conditional Variable Length Traverse";
    condVarLenTraverse->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
    condVarLenTraverse->op.consume = CondVarLenTraverseConsume;
    condVarLenTraverse->op.reset = CondVarLenTraverseReset;
    condVarLenTraverse->op.free = CondVarLenTraverseFree;

    condVarLenTraverse->op.modifies = array_new(uint, 1);
    condVarLenTraverse->op.modifies = array_append(condVarLenTraverse->op.modifies, ae->dest_node_idx);

    return (OpBase*)condVarLenTraverse;
}

Record CondVarLenTraverseConsume(OpBase *opBase) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* Incase we don't have any relations to traverse we can return quickly
     * Consider: MATCH (S)-[:L*]->(M) RETURN M
     * where label L does not exists. */
    if(op->edgeRelationCount == 0) {
        return NULL;
    }

    Path p = NULL;
    while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
        Record childRecord = child->consume(child);
        if(!childRecord) return NULL;

        if(op->r) Record_Free(op->r);
        op->r = childRecord;

        Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);

        AllPathsCtx_Free(op->allPathsCtx);
        op->allPathsCtx = AllPathsCtx_New(srcNode,
                                          op->g,
                                          op->edgeRelationTypes,
                                          op->edgeRelationCount,
                                          op->traverseDir,
                                          op->minHops,
                                          op->maxHops);
    }

    // For the timebeing we only care for the last node in path
    Node n = Path_pop(p);
    Path_free(p);

    Record_AddNode(op->r, op->destNodeIdx, n);
    return Record_Clone(op->r);
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    if(op->r) Record_Free(op->r);
    op->r = NULL;
    AllPathsCtx_Free(op->allPathsCtx);
    op->allPathsCtx = NULL;
    return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    // array_free(op->edgeRelationTypes); // TODO leak
    AlgebraicExpression_Free(op->ae);
    if(op->r) Record_Free(op->r);
    if(op->allPathsCtx) AllPathsCtx_Free(op->allPathsCtx);
}
