/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "./op_var_len_traverse.h"

void _VarLenTraverseOpHop(VarLenTraverse *op) {
    GrB_mxm(
        op->frontier,           // Output
        op->mask,               // Mask
        NULL,                   // Accumulator
        GxB_LOR_LAND_BOOL,      // Semiring
        op->A,                  // First matrix
        op->A,                  // Second matrix
        op->desc                // Descriptor
    );

    // Avoid cycles, add reached nodes to mask.
    GrB_eWiseAdd(op->mask, NULL, NULL, GrB_LOR, op->mask, op->frontier, NULL);
    op->hops++;
}

OpBase* NewVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, float maxHops, Graph *g, const QueryGraph *qg) {
    assert(ae && minHops <= maxHops && g && qg && ae->operand_count == 1);
    VarLenTraverse *varLenTraverse = malloc(sizeof(VarLenTraverse));
    varLenTraverse->g = g;
    varLenTraverse->A = ae->operands[0].operand;
    varLenTraverse->srcNode = ae->src_node;
    varLenTraverse->destNode = ae->dest_node;
    varLenTraverse->hops = 1;
    varLenTraverse->minHops = minHops;
    varLenTraverse->maxHops = maxHops;

    GrB_Matrix_new(&varLenTraverse->frontier, GrB_BOOL, Graph_NodeCount(g), Graph_NodeCount(g));

    // Avoid cycles, add reached nodes to mask.
    GrB_Matrix_new(&varLenTraverse->mask, GrB_BOOL, Graph_NodeCount(g), Graph_NodeCount(g));
    GrB_Matrix_dup(&varLenTraverse->mask, varLenTraverse->A);

    /* Setup matrix multiplication descriptor
     * Mask is complemented, consider [i,j] which were not encountered.
     * Transpose operand if needed. */
    GrB_Descriptor_new(&varLenTraverse->desc);
    GrB_Descriptor_set(varLenTraverse->desc, GrB_MASK, GrB_SCMP);
    if(ae->operands[0].transpose) GrB_Descriptor_set(varLenTraverse->desc, GrB_INP1, GrB_TRAN);

    /* Perform minimum required hops. */
    for(int i = 0; i < minHops; i++) _VarLenTraverseOpHop(varLenTraverse);
    varLenTraverse->it = TuplesIter_new(varLenTraverse->frontier);

    // Set our Op operations
    OpBase_Init(&varLenTraverse->op);
    varLenTraverse->op.name = "Variable Length Traverse";
    varLenTraverse->op.type = OPType_VAR_LEN_TRAVERSE;
    varLenTraverse->op.consume = VarLenTraverseConsume;
    varLenTraverse->op.reset = VarLenTraverseReset;
    varLenTraverse->op.free = VarLenTraverseFree;
    varLenTraverse->op.modifies = NewVector(char*, 2);

    char *modified = NULL;
    modified = QueryGraph_GetNodeAlias(qg, *varLenTraverse->srcNode);
    Vector_Push(varLenTraverse->op.modifies, modified);
    modified = QueryGraph_GetNodeAlias(qg, *varLenTraverse->destNode);
    Vector_Push(varLenTraverse->op.modifies, modified);

    return (OpBase*)varLenTraverse;
}

OpResult VarLenTraverseConsume(OpBase *opBase, QueryGraph* graph) {
    VarLenTraverse *op = (VarLenTraverse*)opBase;
    
    GrB_Index src_id;
    GrB_Index dest_id;

    while(TuplesIter_next(op->it, &dest_id, &src_id) == TuplesIter_DEPLETED) {
        // See if we can advance.
        GrB_Index nvals;
        GrB_Matrix_nvals(&nvals, op->frontier);
        if(nvals == 0 || op->hops >= op->maxHops) return OP_DEPLETED;
        
        _VarLenTraverseOpHop(op);
        TuplesIter_reuse(op->it, op->frontier);
    }

    Node *srcNode = Graph_GetNode(op->g, src_id);
    Node *destNode = Graph_GetNode(op->g, dest_id);
    *op->srcNode = srcNode;
    *op->destNode = destNode;
    return OP_OK;
}

OpResult VarLenTraverseReset(OpBase *ctx) {
    VarLenTraverse *op = (VarLenTraverse*)ctx;
    op->hops = 1;
    GrB_free(&op->mask);
    GrB_Matrix_new(&op->mask, GrB_BOOL, Graph_NodeCount(op->g), Graph_NodeCount(op->g));
    GrB_Matrix_dup(&op->mask, op->A);
    TuplesIter_clear(op->it);

    /* Perform minimum required hops. */
    for(int i = 0; i < op->minHops; i++) _VarLenTraverseOpHop(op);
    TuplesIter_reuse(op->it, op->frontier);

    return OP_OK;
}

void VarLenTraverseFree(OpBase *ctx) {
    VarLenTraverse *op = (VarLenTraverse*)ctx;
    GrB_free(&op->mask);
    GrB_free(&op->frontier);
    GrB_Descriptor_free(&op->desc);
    if(op->it) TuplesIter_free(op->it);
    free(op);
}
