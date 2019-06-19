/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../parser/ast.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

static void _setupTraversedRelations(CondVarLenTraverse *op) {
    AST *ast = op->ast;
    op->relationIDsCount = 0;
    GraphContext *gc = GraphContext_GetFromTLS();    
    AST_LinkEntity *e = (AST_LinkEntity*)MatchClause_GetEntity(ast->matchNode, op->ae->edge->alias);
    int relationIDsCount = AST_LinkEntity_LabelCount(e);

    if(relationIDsCount > 0) {
        op->relationIDs = array_new(int, relationIDsCount);
        for(int i = 0; i < relationIDsCount; i++) {
            Schema *s = GraphContext_GetSchema(gc, e->labels[i], SCHEMA_EDGE);
            if(!s) continue;
            op->relationIDs = array_append(op->relationIDs, s->id);
        }        
    } else {
        op->relationIDs = array_new(int, 1);
        op->relationIDs = array_append(op->relationIDs, GRAPH_NO_RELATION);
    }
    op->relationIDsCount = array_len(op->relationIDs);
}

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g, AST *ast) {
    assert(ae && minHops <= maxHops && g && ae->operand_count == 1);

    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->ae = ae;
    condVarLenTraverse->ast = ast;
    condVarLenTraverse->relationIDs = NULL;
    condVarLenTraverse->srcNodeIdx = AST_GetAliasID(ast, ae->src_node->alias);
    condVarLenTraverse->destNodeIdx = AST_GetAliasID(ast, ae->dest_node->alias);
    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->allPathsCtx = NULL;
    condVarLenTraverse->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING : GRAPH_EDGE_DIR_OUTGOING;
    condVarLenTraverse->r = NULL;

    _setupTraversedRelations(condVarLenTraverse);

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

Record CondVarLenTraverseConsume(OpBase *opBase) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* Incase we don't have any relations to traverse we can return quickly
     * Consider: MATCH (S)-[:L*]->(M) RETURN M
     * where label L does not exists. */
    if(op->relationIDsCount == 0) {
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
                                          op->relationIDs,
                                          op->relationIDsCount,
                                          op->traverseDir,
                                          op->minHops,
                                          op->maxHops);
    }

    // For the timebeing we only care for the last node in path
    Node n = Path_head(p);

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
    array_free(op->relationIDs);
    AlgebraicExpression_Free(op->ae);
    if(op->r) Record_Free(op->r);
    if(op->allPathsCtx) AllPathsCtx_Free(op->allPathsCtx);
}
