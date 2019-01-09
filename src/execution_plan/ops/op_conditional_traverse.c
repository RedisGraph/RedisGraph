/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_conditional_traverse.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"

static void _setupTraversedRelations(CondTraverse *op) {
    AST *ast = AST_GetFromLTS();
    GraphContext *gc = GraphContext_GetFromLTS();
    const char *alias = op->algebraic_expression->edge->alias;
    AST_LinkEntity *e = (AST_LinkEntity*)MatchClause_GetEntity(ast->matchNode, alias);
    op->edgeRelationCount = AST_LinkEntity_LabelCount(e);
    
    if(op->edgeRelationCount > 0) {
        op->edgeRelationTypes = array_new(int , op->edgeRelationCount);
        for(int i = 0; i < op->edgeRelationCount; i++) {
            LabelStore *s = GraphContext_GetStore(gc, e->labels[i], STORE_EDGE);
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

void _extractColumn(CondTraverse *op, const Record r) {
    Node *n = Record_GetNode(r, op->srcNodeRecIdx);
    NodeID srcId = ENTITY_GET_ID(n);

    GrB_Matrix_setElement_BOOL(op->F, true, srcId, 0);

    GrB_Matrix res;
    GrB_Index dim = Graph_RequiredMatrixDim(op->graph);
    GrB_Matrix_new(&res, GrB_BOOL, dim, 1);
    AlgebraicExpression_EvalWithFilter(op->algebraic_expression->exp_root, op->F, res);

    if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
    else GxB_MatrixTupleIter_reuse(op->iter, op->M);

    // Clear filter matrix.
    GxB_Matrix_Delete(op->F, srcId, 0);
}

OpBase* NewCondTraverseOp(Graph *g, AE_Unit *algebraic_expression) {
    CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
    traverse->graph = g;
    traverse->algebraic_expression = algebraic_expression;
    traverse->edgeRelationTypes = NULL;
    traverse->F = NULL;    
    traverse->iter = NULL;
    traverse->edges = NULL;
    
    GrB_Matrix_new(&traverse->M, GrB_BOOL, Graph_RequiredMatrixDim(g), 1);

    AST *ast = AST_GetFromLTS();
    // TODO src and dest not guaranteed to exist, figure out how to handle this
    // assert(traverse->srcNodeRecIdx);
    // assert(traverse->destNodeRecIdx);
    if (algebraic_expression->src) traverse->srcNodeRecIdx = AST_GetAliasID(ast, algebraic_expression->src->alias);
    if (algebraic_expression->dest) traverse->destNodeRecIdx = AST_GetAliasID(ast, algebraic_expression->dest->alias);
    
    // Set our Op operations
    OpBase_Init(&traverse->op);
    traverse->op.name = "Conditional Traverse";
    traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
    traverse->op.consume = CondTraverseConsume;
    traverse->op.reset = CondTraverseReset;
    traverse->op.free = CondTraverseFree;
    traverse->op.modifies = NewVector(char*, 1);

    char *modified = NULL;    
    if (algebraic_expression->dest) modified = algebraic_expression->dest->alias;
    Vector_Push(traverse->op.modifies, modified);

    if(algebraic_expression->edge) {
        _setupTraversedRelations(traverse);
        modified = algebraic_expression->edge->alias;
        Vector_Push(traverse->op.modifies, modified);
        traverse->edges = array_new(Edge, 32);
        traverse->edgeRecIdx = AST_GetAliasID(ast, algebraic_expression->edge->alias);
    }

    return (OpBase*)traverse;
}

/* CondTraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
Record CondTraverseConsume(OpBase *opBase) {
    CondTraverse *op = (CondTraverse*)opBase;
    OpBase *child = op->op.children[0];
    
    /* Not initialized. */
    if(op->iter == NULL) {
        op->r = child->consume(child);
        if(!op->r) return NULL;

        GrB_Matrix_new(&op->F, GrB_BOOL, Graph_RequiredMatrixDim(op->graph), 1);

        /* Pick a column. */
        _extractColumn(op, op->r);
    }

    /* If we're required to update edge,
     * try to get an edge, if successful we can return quickly,
     * otherwise try to get a new pair of source and destination nodes. */
    if(op->algebraic_expression->edge) {
        if(_CondTraverse_SetEdge(op, op->r)) {
            return Record_Clone(op->r);
        }
    }

    bool depleted = false;
    NodeID dest_id = INVALID_ENTITY_ID;

    while(true) {
        GxB_MatrixTupleIter_next(op->iter, &dest_id, NULL, &depleted);

        // Managed to get a tuple, break.
        if(!depleted) break;

        Record childRecord = child->consume(child);
        if(!childRecord) return NULL;

        Record_Free(op->r);
        op->r = childRecord;

        _extractColumn(op, op->r);
    }

    /* Get node from current column. */
    Node *destNode = Record_GetNode(op->r, op->destNodeRecIdx);
    Graph_GetNode(op->graph, dest_id, destNode);

    if(op->algebraic_expression->edge != NULL) {
        // We're guarantee to have at least one edge.
        Node *srcNode;
        Node *destNode;
        // size_t operandCount = op->algebraic_expression->operand_count - 1;

        // if(op->algebraic_expression->operands[operandCount].transpose) {
            // srcNode = Record_GetNode(op->r, op->destNodeRecIdx);
            // destNode = Record_GetNode(op->r, op->srcNodeRecIdx);
        // } else {
            srcNode = Record_GetNode(op->r, op->srcNodeRecIdx);
            destNode = Record_GetNode(op->r, op->destNodeRecIdx);
        // }

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
    if(op->F) {
        GrB_Matrix_free(&op->F);
        op->F = NULL;
    }
    return OP_OK;
}

/* Frees CondTraverse */
void CondTraverseFree(OpBase *ctx) {
    CondTraverse *op = (CondTraverse*)ctx;
    if(op->r) Record_Free(op->r);
    if(op->iter) GxB_MatrixTupleIter_free(op->iter);
    if(op->F) GrB_Matrix_free(&op->F);
    if(op->M) GrB_Matrix_free(&op->M);
    if(op->edges) array_free(op->edges);
    // if(op->algebraic_expression) AlgebraicExpression_Free(op->algebraic_expression);
    if(op->edgeRelationTypes) array_free(op->edgeRelationTypes);
}
