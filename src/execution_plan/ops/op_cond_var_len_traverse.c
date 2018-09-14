/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "./op_cond_var_len_traverse.h"

/* C = MASK(A * B)
 * MASK += C */
void _CondVarLenTraverseOpHop(GrB_Matrix A, GrB_Vector B, GrB_Vector C, GrB_Vector mask, GrB_Descriptor desc) {
    GrB_mxv(
        C,                  // Output
        mask,               // Mask
        NULL,               // Accumulator
        GxB_LOR_LAND_BOOL,  // Semiring
        A,                  // First matrix
        B,                  // Second vector
        desc                // Descriptor
    );

    // Avoid cycles, add reached nodes to mask.
    GrB_eWiseAdd(mask, NULL, NULL, GrB_LOR, mask, C, NULL);    
}

void _VarLenTraverseMinHops(CondVarLenTraverse *op) {
    NodeID src_id = (*op->srcNode)->id;
    assert(src_id != INVALID_ENTITY_ID);

    GrB_Vector v;
    GrB_Vector_new(&v, GrB_BOOL, Graph_NodeCount(op->g));
    GrB_Vector_setElement_BOOL(v, true, src_id);

    // Reset mask.
    GrB_Vector_free(&op->mask);
    GrB_Vector_new(&op->mask, GrB_BOOL, Graph_NodeCount(op->g));

    /* Perform minimum required hops. 
     * C = A*B */
    _CondVarLenTraverseOpHop(op->relation,  // A
                            v,              // B
                            op->frontier,   // C
                            op->mask,
                            op->desc);

    for(int i = 1; i < op->minHops; i++) {
        // C = A*B
        _CondVarLenTraverseOpHop(op->relation,  // A
                                op->frontier,   // B
                                op->frontier,   // C
                                op->mask,
                                op->desc);
        
        // TODO: quick stop if NNZ == 0.
    }
    op->hops = op->minHops;
    TuplesIter_reuse(op->it, (GrB_Matrix)op->frontier);
    GrB_Vector_free(&v);
}

OpBase* NewCondVarLenTraverseOp(AlgebraicExpression *ae, unsigned int minHops, float maxHops, Graph *g, const QueryGraph *qg) {
    assert(ae && minHops <= maxHops && g && qg && ae->operand_count == 1);
    CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
    condVarLenTraverse->g = g;
    condVarLenTraverse->hops = 0;
    condVarLenTraverse->it = NULL;
    condVarLenTraverse->relation = ae->operands[0].operand;
    condVarLenTraverse->srcNode = ae->src_node;
    condVarLenTraverse->destNode = ae->dest_node;
    condVarLenTraverse->minHops = minHops;
    condVarLenTraverse->maxHops = maxHops;

    /* Setup matrix multiplication descriptor
     * Mask is complemented, consider [i,j] which were not encountered.
     * Transpose operand if needed. */
    GrB_Vector_new(&condVarLenTraverse->mask, GrB_BOOL, Graph_NodeCount(condVarLenTraverse->g));
    GrB_Descriptor_new(&condVarLenTraverse->desc);
    GrB_Descriptor_set(condVarLenTraverse->desc, GrB_MASK, GrB_SCMP);
    if(ae->operands[0].transpose) GrB_Descriptor_set(condVarLenTraverse->desc, GrB_INP0, GrB_TRAN);

    GrB_Vector_new(&condVarLenTraverse->frontier, GrB_BOOL, Graph_NodeCount(condVarLenTraverse->g));
    
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
    if(op->it == NULL) {
        res = child->consume(child, graph);
        if(res != OP_OK) return res;
        op->it = TuplesIter_new((GrB_Matrix)op->frontier);
        _VarLenTraverseMinHops(op);
    }

    while(TuplesIter_next(op->it, &dest_id, &src_id) == TuplesIter_DEPLETED) {
        // See if we can advance by hoping.
        GrB_Index nvals;        
        GrB_Vector_nvals(&nvals, op->frontier);
        if(nvals != 0 && op->hops < op->maxHops) {
            // C = A*B
            _CondVarLenTraverseOpHop(op->relation,  // A
                                    op->frontier,   // B
                                    op->frontier,   // C
                                    op->mask,
                                    op->desc);
            op->hops++;
            TuplesIter_reuse(op->it, (GrB_Matrix)op->frontier);
        } else {
            res = child->consume(child, graph);
            if(res != OP_OK) return res;
            _VarLenTraverseMinHops(op);
        }
    }

    Node *destNode = Graph_GetNode(op->g, dest_id);
    *op->destNode = destNode;
    return OP_OK;
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    op->hops = 0;
    TuplesIter_clear(op->it);
    GrB_Vector_free(&op->frontier);
    GrB_Vector_new(&op->frontier, GrB_BOOL, Graph_NodeCount(op->g));
    // TODO: I think Reset should propegate to child nodes.
    return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
    CondVarLenTraverse *op = (CondVarLenTraverse*)ctx;
    GrB_free(&op->mask);
    GrB_free(&op->frontier);
    GrB_Descriptor_free(&op->desc);
    if(op->it) TuplesIter_free(op->it);
}
