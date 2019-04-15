/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../parser/newast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

static void _setupTraversedRelations(CondVarLenTraverse *op) {
    NEWAST *ast = NEWAST_GetFromTLS();
    GraphContext *gc = GraphContext_GetFromTLS();

    uint id = NEWAST_GetAliasID(ast, op->ae->edge->alias);
    AR_ExpNode *exp = NEWAST_GetEntity(ast, id);
    // TODO validate this access
    const cypher_astnode_t *ast_relation = exp->operand.variadic.ast_ref;
    op->relationIDsCount = cypher_ast_rel_pattern_nreltypes(ast_relation);

    if(op->relationIDsCount > 0) {
        op->relationIDs = array_new(int, op->relationIDsCount);
        for(int i = 0; i < op->relationIDsCount; i++) {
            // TODO documentation error in libcypher-parser
            const cypher_astnode_t *ast_reltype = cypher_ast_rel_pattern_get_reltype(ast_relation, i);  
            const char *reltype = cypher_ast_reltype_get_name(ast_reltype);
            Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
            if(!s) continue;
            op->relationIDs = array_append(op->relationIDs, s->id);
        }        
    } else {
        op->relationIDs = array_new(int, 1);
        op->relationIDs = array_append(op->relationIDs, GRAPH_NO_RELATION);
    }
    op->relationIDsCount = array_len(op->relationIDs);
}

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g) {
    assert(ae && minHops <= maxHops && g && ae->operand_count == 1);

    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->ae = ae;
    NEWAST *ast = NEWAST_GetFromTLS();
    condVarLenTraverse->relationIDs = NULL;
    condVarLenTraverse->srcNodeIdx = NEWAST_GetAliasID(ast, ae->src_node->alias);
    condVarLenTraverse->destNodeIdx = NEWAST_GetAliasID(ast, ae->dest_node->alias);
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
    array_free(op->relationIDs);
    AlgebraicExpression_Free(op->ae);
    if(op->r) Record_Free(op->r);
    if(op->allPathsCtx) AllPathsCtx_Free(op->allPathsCtx);
}
