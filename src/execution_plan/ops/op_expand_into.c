/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"

// String representation of operation.
static int ExpandIntoToString(const OpBase *ctx, char *buff, uint buff_len) {
    const OpExpandInto *op = (const OpExpandInto*)ctx;

    int offset = 0;    
    offset += snprintf(buff + offset, buff_len-offset, "%s | ", op->op.name);
    offset += Node_ToString(op->ae->src_node, buff + offset, buff_len - offset);
    if(op->ae->edge) {
        offset += snprintf(buff + offset, buff_len-offset, "-");
        offset += Edge_ToString(op->ae->edge, buff + offset, buff_len - offset);
        offset += snprintf(buff + offset, buff_len-offset, "->");
    } else {
        offset += snprintf(buff + offset, buff_len-offset, "->");
    }
    offset += Node_ToString(op->ae->dest_node, buff + offset, buff_len - offset);
    return offset;
}

/* Collects traversed edge relations.
 * e.g. [e:R0|R1]
 * op->edgeRelationTypes will hold both R0 and R1 IDs.
 * in the case where no relationship types are specified
 * op->edgeRelationTypes will contain GRAPH_NO_RELATION. */
static void _setupTraversedRelations(OpExpandInto *op, GraphContext *gc) {
    AST *ast = op->ast;
    const char *alias = op->ae->edge->alias;
    AST_LinkEntity *e = (AST_LinkEntity*)MatchClause_GetEntity(ast->matchNode, alias);
    op->edgeRelationCount = AST_LinkEntity_LabelCount(e);
    
    if(op->edgeRelationCount > 0) {
        op->edgeRelationTypes = array_new(int, op->edgeRelationCount);
        for(int i = 0; i < op->edgeRelationCount; i++) {
            Schema *s = GraphContext_GetSchema(gc, e->labels[i], SCHEMA_EDGE);
            if(!s) continue;
            op->edgeRelationTypes = array_append(op->edgeRelationTypes, s->id);
        }
    } else {
        op->edgeRelationCount = 1;
        op->edgeRelationTypes = array_new(int, 1);
        op->edgeRelationTypes = array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
    }
    op->edgeRelationCount = array_len(op->edgeRelationTypes);
}

// Sets traversed edge within record.
static bool _setEdge(OpExpandInto *op) {
    // Consumed edges connecting current source and destination nodes.
    if(!array_len(op->edges)) return false;

    Edge *e = op->edges + (array_len(op->edges)-1);
    Record_AddEdge(op->r, op->edgeRecIdx, *e);
    array_pop(op->edges);
    return true;
}

/* Determin the maximum number of records
 * which will be considered when evaluating an algebraic expression. */
static uint _determinRecordCap(const AST *ast) {
    uint recordsCap = 16;    // Default.
    if(ast->limitNode) recordsCap = MIN(recordsCap, ast->limitNode->limit);
    return recordsCap;
}

/* Evaluate algebraic expression:
 * appends filter matrix as the left most operand
 * perform multiplications.
 * removed filter matrix from original expression
 * clears filter matrix. */
static void _traverse(OpExpandInto *op) {
    // Append filter matrix to algebraic expression, as the left most operand.
    AlgebraicExpression_PrependTerm(op->ae, op->F, false, false, false);

    // Evaluate expression.
    AlgebraicExpression_Execute(op->ae, op->M);
    
    // Remove operand.
    AlgebraicExpression_RemoveTerm(op->ae, 0, NULL);

    // Clear filter matrix.
    GrB_Matrix_clear(op->F);
}

OpBase* NewExpandIntoOp(AlgebraicExpression *ae, AST *ast) {
    OpExpandInto *expandInto = calloc(1, sizeof(OpExpandInto));
    GraphContext *gc = GraphContext_GetFromTLS();
    expandInto->ae = ae;
    expandInto->F = NULL;
    expandInto->r = NULL;
    expandInto->ast = ast;
    expandInto->edges = NULL;
    expandInto->graph = gc->g;
    expandInto->recordsLen = 0;
    expandInto->edgeRelationTypes = NULL;
    expandInto->recordsCap = _determinRecordCap(ast);
    expandInto->srcNodeRecIdx = AST_GetAliasID(ast, ae->src_node->alias);
    expandInto->destNodeRecIdx = AST_GetAliasID(ast, ae->dest_node->alias);
    expandInto->records = rm_calloc(expandInto->recordsCap, sizeof(Record));

    // Set our Op operations
    OpBase_Init(&expandInto->op);
    expandInto->op.name = "Expand Into";
    expandInto->op.type = OPType_EXPAND_INTO;
    expandInto->op.init = ExpandIntoInit;
    expandInto->op.free = ExpandIntoFree;
    expandInto->op.reset = ExpandIntoReset;
    expandInto->op.consume = ExpandIntoConsume;
    expandInto->op.toString = ExpandIntoToString;
    expandInto->op.modifies = NULL;

    if(ae->edge) {
        char *modified = NULL;
        _setupTraversedRelations(expandInto, gc);
        modified = expandInto->ae->edge->alias;
        expandInto->op.modifies = NewVector(char*, 0);
        Vector_Push(expandInto->op.modifies, modified);
        expandInto->edges = array_new(Edge, 32);
        expandInto->edgeRecIdx = AST_GetAliasID(ast, ae->edge->alias);
    }

    return (OpBase*)expandInto;
}

OpResult ExpandIntoInit(OpBase *opBase) {
    OpExpandInto *op = (OpExpandInto*)opBase;

    size_t required_dim = Graph_RequiredMatrixDim(op->graph);
    GrB_Matrix_new(&op->M, GrB_BOOL, op->recordsCap, required_dim);
    GrB_Matrix_new(&op->F, GrB_BOOL, op->recordsCap, required_dim);
    return OP_OK;
}

/* Emits a record when possible,
 * Returns NULL when we've got no more records. */
static Record _handoff(OpExpandInto *op) {
    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->ae->edge) {
        if(_setEdge(op)) return Record_Clone(op->r);
    }

    Node *srcNode;
    Node *destNode;
    NodeID srcId = INVALID_ENTITY_ID;
    NodeID destId = INVALID_ENTITY_ID;

    /* Find a record where both record's source and destination
     * nodes are connected. */
    while(op->recordsLen) {
        op->recordsLen--;
        op->r = op->records[op->recordsLen];

        srcNode = Record_GetNode(op->r, op->srcNodeRecIdx);
        destNode = Record_GetNode(op->r, op->destNodeRecIdx);
        srcId = ENTITY_GET_ID(srcNode);
        destId = ENTITY_GET_ID(destNode);

        bool x;
        GrB_Info res = GrB_Matrix_extractElement_BOOL(&x, op->M, srcId, destId);
        // Src is not connected to dest.
        if(res != GrB_SUCCESS) continue;

        // If we're here src is connected to dest.
        if(op->ae->edge) {
            for(int i = 0; i < op->edgeRelationCount; i++) {
                Graph_GetEdgesConnectingNodes(op->graph,
                                            srcId,
                                            destId,
                                            op->edgeRelationTypes[i],
                                            &op->edges);
            }
            _setEdge(op);
            return Record_Clone(op->r);
        }

        // Mark as NULL to avoid double free.
        op->records[op->recordsLen] = NULL;
        return op->r;
    }

    // Didn't manage to emit record.
    return NULL;
}

/* ExpandIntoConsume next operation 
 * returns OP_DEPLETED when no additional updates are available */
Record ExpandIntoConsume(OpBase *opBase) {
    Node *n;
    Record r;
    OpExpandInto *op = (OpExpandInto*)opBase;
    OpBase *child = op->op.children[0];

    // As long as we don't have a record to emit.
    while((r = _handoff(op)) == NULL) {
        /* If we're here, we didn't managed to emit a record,
         * clean up and try to get new data points. */
        for(int i = 0; i < op->recordsCap; i++) {
            if(op->records[i]) {
                Record_Free(op->records[i]);
                op->records[i] = NULL;
            } else {
                break;
            }
        }

        // Ask child operations for data.
        for(op->recordsLen = 0; op->recordsLen < op->recordsCap; op->recordsLen++) {
            Record childRecord = child->consume(child);
            // Did not managed to get new data, break.
            if(!childRecord) break;

            // Store received record.
            op->records[op->recordsLen] = childRecord;
            /* Update filter matrix F, set row i at position srcId
             * F[i, srcId] = true. */
            n = Record_GetNode(childRecord, op->srcNodeRecIdx);
            NodeID srcId = ENTITY_GET_ID(n);
            GrB_Matrix_setElement_BOOL(op->F, true, op->recordsLen, srcId);
        }

        // Depleted.
        if(op->recordsLen == 0) return NULL;
        _traverse(op);
    }

    return r;
}

OpResult ExpandIntoReset(OpBase *ctx) {
    OpExpandInto *op = (OpExpandInto*)ctx;
    for(int i = 0; i < op->recordsLen; i++) {
        if(op->records[i]) Record_Free(op->records[i]);
        op->records = NULL;
    }
    op->recordsLen = 0;

    if(op->F) GrB_Matrix_clear(op->F);
    if(op->edges) array_clear(op->edges);
    return OP_OK;
}

/* Frees ExpandInto */
void ExpandIntoFree(OpBase *ctx) {
    OpExpandInto *op = (OpExpandInto*)ctx;
    if(op->F) GrB_Matrix_free(&op->F);
    if(op->M) GrB_Matrix_free(&op->M);
    if(op->edges) array_free(op->edges);
    if(op->edgeRelationTypes) array_free(op->edgeRelationTypes);
    if(op->ae) AlgebraicExpression_Free(op->ae);
    if(op->records) {
        for(int i = 0; i < op->recordsCap; i++) {
            if(op->records[i]) Record_Free(op->records[i]);
            else break;
        }
        rm_free(op->records);
    }
}
