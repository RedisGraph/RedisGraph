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

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, unsigned int maxHops, Graph *g, const QueryGraph *qg) {
    assert(ae && minHops <= maxHops && g && qg && ae->operand_count == 1);
    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->relationID = Edge_GetRelationID(*ae->edge);
    condVarLenTraverse->srcNode = ae->src_node;
    condVarLenTraverse->destNode = ae->dest_node;
    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;
    condVarLenTraverse->paths = array_new(Path, 32);

    // Set our Op operations
    OpBase_Init(&condVarLenTraverse->op);
    condVarLenTraverse->op.name = "Conditional Variable Length Traverse";
    condVarLenTraverse->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
    condVarLenTraverse->op.consume = CondVarLenTraverseConsume;
    condVarLenTraverse->op.reset = CondVarLenTraverseReset;
    condVarLenTraverse->op.free = CondVarLenTraverseFree;
    condVarLenTraverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *condVarLenTraverse->srcNode);
    Vector_Push(condVarLenTraverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *condVarLenTraverse->destNode);
    Vector_Push(condVarLenTraverse->op.modifies, modified);

    return (OpBase*)condVarLenTraverse;
}

OpResult CondVarLenTraverseConsume(OpBase *opBase, QueryGraph* graph) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)opBase;
    OpBase *child = op->op.children[0];

    OpResult res;
    GrB_Index src_id;
    GrB_Index dest_id;

    /* Not initialized. */
    while(array_len(op->paths) == 0) {
        res = child->consume(child, graph);
        if(res != OP_OK) return res;
        printf("op->maxHops: %d\n", op->maxHops);
        AllPaths(op->g, op->relationID, (*op->srcNode)->id, op->minHops, op->maxHops, &op->paths);
    }

    Path p = array_pop(op->paths);
    Edge *e = Path_pop(p);
    Path_free(p);

    Node *destNode = Edge_GetDestNode(e);
    *op->destNode = destNode;
    return OP_OK;
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    // Empty paths.
    size_t pathCount = array_len(op->paths);
    for(int i = pathCount-1; i >= 0; i--) {
        Path p = op->paths[i];
        Path_free(p);
        array_pop(op->paths);
    }

    // TODO: I think Reset should propegate to child nodes.
    return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    array_free(op->paths);
}
