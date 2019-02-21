/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"
#include "../../parser/newast.h"
#include "../../arithmetic/arithmetic_expression.h"

static void _setupTraversedRelations(CondTraverse *op) {
    NEWAST *ast = NEWAST_GetFromTLS();
    GraphContext *gc = GraphContext_GetFromTLS();
    char *alias = op->algebraic_expression->edge->alias;
    
    unsigned int id = NEWAST_GetAliasID(ast, alias);
    AR_ExpNode *e = NEWAST_GetEntity(ast, id);
    const cypher_astnode_t *ast_entity = e->operand.variadic.ast_ref;
    op->edgeRelationCount = cypher_ast_rel_pattern_nreltypes(ast_entity);
    if(op->edgeRelationCount > 0) {
        op->edgeRelationTypes = array_new(int , op->edgeRelationCount);
        for(int i = 0; i < op->edgeRelationCount; i++) {
            const char *label = cypher_ast_reltype_get_name(cypher_ast_rel_pattern_get_reltype(ast_entity, i));
            Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
            if(!s) continue;
            op->edgeRelationTypes = array_append(op->edgeRelationTypes, s->id);
        }
    } else {
        op->edgeRelationCount = 1;
        op->edgeRelationTypes = array_new(int , 1);
        op->edgeRelationTypes = array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
    }
}

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
    AlgebraicExpression_AppendTerm(op->algebraic_expression, op->F, false, false);

    // Evaluate expression.
    AlgebraicExpression_Execute(op->algebraic_expression, op->M);
    
    // Remove operand.
    AlgebraicExpression_RemoveTerm(op->algebraic_expression, op->algebraic_expression->operand_count-1, NULL);

    if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
    else GxB_MatrixTupleIter_reuse(op->iter, op->M);

    // Clear filter matrix.
    GrB_Matrix_clear(op->F);
}

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
static int _determinRecordCap(const NEWAST *ast) {
    int recordsCap = 16;    // Default.
    const cypher_astnode_t *ret_clause = NEWAST_GetClause(ast->root, CYPHER_AST_RETURN);
    if (ret_clause == NULL) return recordsCap;
    // TODO should just store this number somewhere, as this logic is also in resultset
    const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);
    if (limit_clause) {
        int limit = NEWAST_ParseIntegerNode(limit_clause);
        recordsCap = MIN(recordsCap, limit);
    }
    return recordsCap;
}

OpBase* NewCondTraverseOp(Graph *g, AlgebraicExpression *algebraic_expression, NEWAST *ast) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->ast = ast;
    traverse->graph = g;
    traverse->algebraic_expression = algebraic_expression;
    traverse->edgeRelationTypes = NULL;
    traverse->F = NULL;    
    traverse->iter = NULL;
    traverse->edges = NULL;
    traverse->r = NULL;    

    traverse->srcNodeRecIdx = NEWAST_GetAliasID(ast, algebraic_expression->src_node->alias);
    traverse->destNodeRecIdx = NEWAST_GetAliasID(ast, algebraic_expression->dest_node->alias);
    
    traverse->recordsLen = 0;
    traverse->transposed_edge = false;
    traverse->recordsCap = _determinRecordCap(ast);
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
    traverse->op.modifies = NewVector(char*, 1);

    char *modified = NULL;    
    modified = traverse->algebraic_expression->dest_node->alias;
    Vector_Push(traverse->op.modifies, modified);

    if(algebraic_expression->edge) {
        _setupTraversedRelations(traverse);
        modified = traverse->algebraic_expression->edge->alias;
        Vector_Push(traverse->op.modifies, modified);
        traverse->edges = array_new(Edge, 32);
        traverse->edgeRecIdx = NEWAST_GetAliasID(ast, algebraic_expression->edge->alias);
    }

    return (OpBase*)traverse;
}

OpResult CondTraverseInit(OpBase *opBase) {
    CondTraverse *op = (CondTraverse*)opBase;
    size_t op_idx = op->algebraic_expression->operand_count - 1;
    AlgebraicExpression *exp = op->algebraic_expression;
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
    if(op->algebraic_expression->edge) {
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
            Node *n = Record_GetNode(childRecord, op->srcNodeRecIdx);
            NodeID srcId = ENTITY_GET_ID(n);
            GrB_Matrix_setElement_BOOL(op->F, true, srcId, op->recordsLen);
        }

        // No data.
        if(op->recordsLen == 0) return NULL;

        _traverse(op);
    }

    /* Get node from current column. */
    op->r = op->records[src_id];
    Node *destNode = Record_GetNode(op->r, op->destNodeRecIdx);
    Graph_GetNode(op->graph, dest_id, destNode);

    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        Node *srcNode;
        Node *destNode;

        if(op->transposed_edge) {
            srcNode = Record_GetNode(op->r, op->destNodeRecIdx);
            destNode = Record_GetNode(op->r, op->srcNodeRecIdx);
        } else {
            srcNode = Record_GetNode(op->r, op->srcNodeRecIdx);
            destNode = Record_GetNode(op->r, op->destNodeRecIdx);
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
    if(op->algebraic_expression) AlgebraicExpression_Free(op->algebraic_expression);
    if(op->edgeRelationTypes) array_free(op->edgeRelationTypes);
    if(op->records) {
        for(int i = 0; i < op->recordsLen; i++) Record_Free(op->records[i]);
        rm_free(op->records);
    }
}
