/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../ast/ast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

static void _setupTraversedRelations(CondVarLenTraverse *op, QGEdge *e) {
    uint reltype_count = array_len(e->reltypeIDs);
    if (reltype_count > 0) {
        op->edgeRelationTypes = e->reltypeIDs; // TODO can't free this way
        op->edgeRelationCount = reltype_count;
        // op->edgeRelationTypes = array_new(int , op->edgeRelationCount);
        // for(int i = 0; i < op->edgeRelationCount; i++) {
            // Schema *s = GraphContext_GetSchema(gc, e->labels[i], SCHEMA_EDGE);
            // if(!s) continue;
            // op->edgeRelationTypes = array_append(op->edgeRelationTypes, s->id);
        // }
        // op->edgeRelationCount = array_len(op->edgeRelationTypes);
    
    } else {
        op->edgeRelationCount = 1;
        op->edgeRelationTypes = array_new(int, 1);
        op->edgeRelationTypes = array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
    }
}

int CondVarLenTraverseToString(const OpBase *ctx, char *buff, uint buff_len) {
    const CondVarLenTraverse *op = (const CondVarLenTraverse*)ctx;

    int offset = 0;    
    offset += snprintf(buff + offset, buff_len-offset, "%s | ", op->op.name);
    offset += QGNode_ToString(op->ae->src_node, buff + offset, buff_len - offset);
    if(op->ae->edge) {
        offset += snprintf(buff + offset, buff_len-offset, "-");
        offset += QGEdge_ToString(op->ae->edge, buff + offset, buff_len - offset);
        offset += snprintf(buff + offset, buff_len-offset, "->");
    } else {
        offset += snprintf(buff + offset, buff_len-offset, "->");
    }
    offset += QGNode_ToString(op->ae->dest_node, buff + offset, buff_len - offset);
    return offset;
}

void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op) {
    // Expand into doesn't performs any modifications.
    // Vector_Clear(op->op.modifies);
    op->expandInto = true;
    op->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO;
    op->op.name = "Conditional Variable Length Traverse (Expand Into)";
}

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, uint minHops, uint maxHops, uint src_node_idx, uint dest_node_idx, Graph *g) {
    assert(ae && minHops <= maxHops && g && ae->operand_count == 1);

    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->ae = ae;
    condVarLenTraverse->expandInto = false;
    // condVarLenTraverse->relationIDs = NULL;
    condVarLenTraverse->edgeRelationTypes = NULL;

    condVarLenTraverse->srcNodeIdx = src_node_idx;
    condVarLenTraverse->destNodeIdx = dest_node_idx;

    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->allPathsCtx = NULL;
    condVarLenTraverse->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING : GRAPH_EDGE_DIR_OUTGOING;
    condVarLenTraverse->r = NULL;

    _setupTraversedRelations(condVarLenTraverse, ae->edge);

    // Set our Op operations
    OpBase_Init(&condVarLenTraverse->op);
    condVarLenTraverse->op.name = "Conditional Variable Length Traverse";
    condVarLenTraverse->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
    condVarLenTraverse->op.consume = CondVarLenTraverseConsume;
    condVarLenTraverse->op.reset = CondVarLenTraverseReset;
    condVarLenTraverse->op.toString = CondVarLenTraverseToString;
    condVarLenTraverse->op.free = CondVarLenTraverseFree;

    condVarLenTraverse->op.modifies = array_new(uint, 1);
    condVarLenTraverse->op.modifies = array_append(condVarLenTraverse->op.modifies, dest_node_idx);

    return (OpBase*)condVarLenTraverse;
}

Record CondVarLenTraverseConsume(OpBase *opBase) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];
    Path p = NULL;

    /* Incase we don't have any relations to traverse we can return quickly
     * Consider: MATCH (S)-[:L*]->(M) RETURN M
     * where label L does not exists. */
    if(op->edgeRelationCount == 0) {
        return NULL;
    }

compute_path:
    while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
        Record childRecord = OpBase_Consume(child);
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
    Node n = Path_head(p);

    if(op->expandInto) {
        /* Dest node is already resolved
         * need to make sure src is connected to dest
         * i.e. n == dest. */
        Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
        if(ENTITY_GET_ID(&n) != ENTITY_GET_ID(destNode)) goto compute_path;
    } else {
        Record_AddNode(op->r, op->destNodeIdx, n);
    }

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
    array_free(op->edgeRelationTypes);
    AlgebraicExpression_Free(op->ae);
    if(op->r) Record_Free(op->r);
    if(op->allPathsCtx) AllPathsCtx_Free(op->allPathsCtx);
}
