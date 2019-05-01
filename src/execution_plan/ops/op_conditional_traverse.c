/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"
#include "../../arithmetic/arithmetic_expression.h"

// Updates query graph edge.
static int _CondTraverse_SetEdge(CondTraverse *op, Record r) {
    // Consumed edges connecting current source and destination nodes.
    if(!array_len(op->edges)) return 0;

    Edge *e = op->edges + (array_len(op->edges)-1);
    Record_AddEdge(r, op->edgeRecIdx, *e);
    array_pop(op->edges);
    return 1;
}

/* Evaluate algebraic expression:
 * appends filter matrix as the right most operand
 * perform multiplications
 * set iterator over result matrix
 * removed filter matrix from original expression
 * clears filter matrix. */
void _traverse(CondTraverse *op) {
    // Append matrix to algebraic expression, as the right most operand.
    AlgebraicExpression_AppendTerm(op->ae, op->F, false, false);

    // Evaluate expression.
    AlgebraicExpression_Execute(op->ae, op->M);

    // Remove operand.
    AlgebraicExpression_RemoveTerm(op->ae, op->ae->operand_count-1, NULL);

    if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
    else GxB_MatrixTupleIter_reuse(op->iter, op->M);

    // Clear filter matrix.
    GrB_Matrix_clear(op->F);
}

OpBase* NewCondTraverseOp(Graph *g, AlgebraicExpression *ae, uint records_cap) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->graph = g;
    traverse->ae = ae;
    traverse->edgeRelationTypes = NULL;
    traverse->F = NULL;
    traverse->iter = NULL;
    traverse->edges = NULL;
    traverse->r = NULL;

    traverse->srcNodeIdx = ae->src_node_idx;
    traverse->destNodeIdx = ae->dest_node_idx;

    traverse->recordsLen = 0;
    traverse->transposed_edge = false;
    traverse->recordsCap = records_cap;
    traverse->records = rm_calloc(traverse->recordsCap, sizeof(Record));
    GrB_Matrix_new(&traverse->M, GrB_BOOL, Graph_RequiredMatrixDim(g), traverse->recordsCap);
    GrB_Matrix_new(&traverse->F, GrB_BOOL, Graph_RequiredMatrixDim(g), traverse->recordsCap);

    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Conditional Traverse";
    traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
    traverse->op.consume = CondTraverseConsume;
    traverse->op.init = CondTraverseInit;
    traverse->op.reset = CondTraverseReset;
    traverse->op.free = CondTraverseFree;
    traverse->op.modifies = array_new(uint, 1);
    traverse->op.modifies = array_append(traverse->op.modifies, ae->dest_node_idx);

    if(ae->edge) {
        traverse->edgeRelationTypes = traverse->ae->relation_ids;
        traverse->edgeRelationCount = array_len(traverse->edgeRelationTypes);
        uint id = traverse->ae->edge_idx;
        if (id != NOT_IN_RECORD) traverse->op.modifies = array_append(traverse->op.modifies, id);
        traverse->edges = array_new(Edge, 32);
        traverse->edgeRecIdx = ae->edge_idx;
    }

    return (OpBase*)traverse;
}

OpResult CondTraverseInit(OpBase *opBase) {
    CondTraverse *op = (CondTraverse*)opBase;
    size_t op_idx = op->ae->operand_count - 1;
    AlgebraicExpression *exp = op->ae;
    // If the input is set to be transposed on the first expression evaluation,
    // the source and destination nodes will be swapped in the record.
    op->transposed_edge = exp->edge && exp->operands[op_idx].transpose;

    return OP_OK;
}

/* CondTraverseConsume next operation
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
Record CondTraverseConsume(OpBase *opBase) {
    CondTraverse *op = (CondTraverse*)opBase;
    OpBase *child = op->op.children[0];

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->ae->edge) {
        if(_CondTraverse_SetEdge(op, op->r)) {
            return Record_Clone(op->r);
        }
    }

    bool depleted = true;
    NodeID src_id = INVALID_ENTITY_ID;
    NodeID dest_id = INVALID_ENTITY_ID;

    while(true) {
        if(op->iter) GxB_MatrixTupleIter_next(op->iter, &dest_id, &src_id, &depleted);

        // Managed to get a tuple, break.
        if(!depleted) break;

        /* Run out of tuples, try to get new data.
         * Free old records. */
        op->r = NULL;
        for(int i = 0; i < op->recordsLen; i++) Record_Free(op->records[i]);

        // Ask child operations for data.
        for(op->recordsLen = 0; op->recordsLen < op->recordsCap; op->recordsLen++) {
            Record childRecord = child->consume(child);
            if(!childRecord) break;

            // Store received record.
            op->records[op->recordsLen] = childRecord;
            /* Update filter matrix F, set column i at position srcId
             * F[srcId, i] = true. */
            Node *n = Record_GetNode(childRecord, op->srcNodeIdx);
            NodeID srcId = ENTITY_GET_ID(n);
            GrB_Matrix_setElement_BOOL(op->F, true, srcId, op->recordsLen);
        }

        // No data.
        if(op->recordsLen == 0) return NULL;

        _traverse(op);
    }

    /* Get node from current column. */
    op->r = op->records[src_id];
    Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
    Graph_GetNode(op->graph, dest_id, destNode);

    if(op->ae->edge != NULL) {
        // We're guarantee to have at least one edge.
        Node *srcNode;
        Node *destNode;

        if(op->transposed_edge) {
            srcNode = Record_GetNode(op->r, op->destNodeIdx);
            destNode = Record_GetNode(op->r, op->srcNodeIdx);
        } else {
            srcNode = Record_GetNode(op->r, op->srcNodeIdx);
            destNode = Record_GetNode(op->r, op->destNodeIdx);
        }

        for(int i = 0; i < op->edgeRelationCount; i++) {
            Graph_GetEdgesConnectingNodes(op->graph,
                                        ENTITY_GET_ID(srcNode),
                                        ENTITY_GET_ID(destNode),
                                        op->edgeRelationTypes[i],
                                        &op->edges);
        }

        _CondTraverse_SetEdge(op, op->r);
    }

    return Record_Clone(op->r);
}

OpResult CondTraverseReset(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    if(op->r) Record_Free(op->r);
    if(op->edges) array_clear(op->edges);
    if(op->iter) {
        GxB_MatrixTupleIter_free(op->iter);
        op->iter = NULL;
    }
    if(op->F) GrB_Matrix_clear(op->F);
    return OP_OK;
}

/* Frees CondTraverse */
void CondTraverseFree(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    if(op->iter) GxB_MatrixTupleIter_free(op->iter);
    if(op->F) GrB_Matrix_free(&op->F);
    if(op->M) GrB_Matrix_free(&op->M);
    if(op->edges) array_free(op->edges);
    if(op->ae) AlgebraicExpression_Free(op->ae);
    if(op->edgeRelationTypes) array_free(op->edgeRelationTypes);
    if(op->records) {
        for(int i = 0; i < op->recordsLen; i++) Record_Free(op->records[i]);
        rm_free(op->records);
    }
}
