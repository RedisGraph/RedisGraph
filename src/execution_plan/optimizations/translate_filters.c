/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "translate_filters.h"
#include "../../parser/grammar.h"
#include "../ops/op_filter.h"
#include "../ops/op_traverse.h"
#include "../ops/op_conditional_traverse.h"
#include <assert.h>

/* Separates filters into two groups:
 * 1. filters applied to the first term rows (left hand)
 * 2. filters applied to the last term columns (right hand) */
void _groupFilters(Vector *filters, const char *srcNodeAlias, const char *destNodeAlias, FT_FilterNode **leftHand, FT_FilterNode **rightHand) {
    for(int i = 0; i < Vector_Size(filters); i++) {
        FT_FilterNode *filter;
        Vector_Get(filters, i , &filter);
        assert(filter->t == FT_N_PRED && filter->pred.t == FT_N_CONSTANT);

        // Determin if filter aplies to source node or dest node.
        FT_FilterNode **hand;        
        if(strcmp(filter->pred.Lop.alias, srcNodeAlias) == 0) {
            hand = leftHand;
        } else {
            hand = rightHand;
        }
        
        // Extend filter by ANDing.
        if(*hand == NULL) {
            *hand = filter;
        } else {
            FT_FilterNode *and = CreateCondFilterNode(AND);
            AppendLeftChild(and, filter);
            AppendRightChild(and, *hand);
            *hand = and;
        }
    }
}

GrB_Matrix _filtersToMatrix(FT_FilterNode *filter, AlgebraicExpressionOperand *term, bool transpose, const Graph *g, Node **filteredEntity) {
    GrB_Matrix M = term->operand;
    
    // Determin which columns are active.
    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);

    /* Matrix reduction is performed on rows 
     * while we're interested in reducing columns
     * and so if the term is transposed then we're all set
     * otherwise we have to transpose. */
    if(transpose) {
        GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
    }
    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, M);

    GrB_Vector activeEntries;
    GrB_Vector_new(&activeEntries, GrB_BOOL, nrows);
    GrB_Matrix_reduce_BinaryOp(activeEntries, NULL, NULL, GrB_LOR, M, desc);
    GrB_Descriptor_free(&desc);

    GrB_Index nvals;
    GrB_Vector_nvals(&nvals, activeEntries);

    // Create filter matrix.    
    GrB_Matrix filterMatrix;
    GrB_Matrix_new(&filterMatrix, GrB_BOOL, nrows, nrows);

    // Scan active entries, apply filter to each entry.
    GrB_Index entryIdx;
    Node *originalNode = *filteredEntity;
    TuplesIter *iter = TuplesIter_new((GrB_Matrix)activeEntries);
    while(TuplesIter_next(iter, &entryIdx, NULL) != TuplesIter_DEPLETED) {
        Node *node = Graph_GetNode(g, entryIdx);
        *filteredEntity = node;
        if(FilterTree_applyFilters(filter)) {
            GrB_Matrix_setElement_BOOL(filterMatrix, true, entryIdx, entryIdx);
        }
    }

    TuplesIter_free(iter);
    GrB_Vector_free(&activeEntries);
    
    // Restore filtered entity node.
    *filteredEntity = originalNode;
    
    return filterMatrix;
}

void _applyLeftHandFilters(FT_FilterNode *filter, AlgebraicExpression *ae, const Graph *g) {
    if(!filter) return;
    AlgebraicExpressionOperand *term = &(ae->operands[0]);
    GrB_Matrix filterMatrix = _filtersToMatrix(filter, term, term->transpose, g, ae->src_node);

    // Prepend filter matrix to algebraic expression.
    AlgebraicExpression_PrependTerm(ae, filterMatrix, false);
}

/* Filter last algebraic expression term columns by applying filter */
void _applyRightHandFilters(FT_FilterNode *filter, AlgebraicExpression *ae, const Graph *g) {
    if(!filter) return;
    AlgebraicExpressionOperand *term = &(ae->operands[ae->operand_count-1]);
    GrB_Matrix filterMatrix = _filtersToMatrix(filter, term, !term->transpose, g, ae->dest_node);

    // Append filter matrix to algebraic expression.
    AlgebraicExpression_AppendTerm(ae, filterMatrix, false);
}

/* Populate filterOps vector with execution plan filter operations
 * applied to given traverseOp. */
void _locateTraverseFilters(const OpNode *traverseOp, Vector *filterOps) {
    assert(traverseOp->operation->type == OPType_TRAVERSE ||
           traverseOp->operation->type == OPType_CONDITIONAL_TRAVERSE);

    /* Scanning execution plan from traverseOp "downwards" using parent pointer
     * until we hit a none filter operation. */
    OpNode *current = traverseOp->parent;
    while(current->operation->type == OPType_FILTER) {
        /* We're only interested in filters which apply to a single entity,
         * Extract modified aliases from filter tree. */
        Filter *filterOp = (Filter*)current->operation;
        FT_FilterNode *filterTree = filterOp->filterTree;
        Vector *filteredEntities = FilterTree_CollectAliases(filterTree);

        /* At this point we're promised that if a filter operation is
         * applied to a single entity E, then E is modified by given traverse op. */
        if(Vector_Size(filteredEntities) == 1) {
            Vector_Push(filterOps, current);
        }
        Vector_Free(filteredEntities);

        // Advance to the next operation.
        current = current->parent;
    }
}

// Populate traverseOps vector with execution plan traverse operations.
void _locateTraverseOp(OpNode *root, Vector *traverseOps) {

    // Is this a traverse operation?
    if(root->operation->type == OPType_TRAVERSE ||
       root->operation->type == OPType_CONDITIONAL_TRAVERSE) {
           Vector_Push(traverseOps, root);
    }

    // Continue scanning.
    for(int i = 0; i < root->childCount; i++) {        
        _locateTraverseOp(root->children[i], traverseOps);
    }
}

void translateFilters(ExecutionPlan *plan) {
    // Get a list of all traverse operations within execution plan.
    Vector *traverseOps = NewVector(OpNode*, 0);
    _locateTraverseOp(plan->root, traverseOps);

    /* For each traverse op, locate filters applied 
     * to their modified entities. */
    OpNode *traverseOp;
    Vector *filterOps = NewVector(OpNode*, 0);

    while(Vector_Pop(traverseOps, &traverseOp)) {
        _locateTraverseFilters(traverseOp, filterOps);
        if(Vector_Size(filterOps) == 0) continue;

        // Extract actual filter trees.
        Vector *filters = NewVector(FT_FilterNode*, Vector_Size(filterOps));
        for(int i = 0; i < Vector_Size(filterOps); i++) {            
            OpNode *opFilter;
            Vector_Get(filterOps, i, &opFilter);
            Filter *f = (Filter *)opFilter->operation;
            Vector_Push(filters, f->filterTree);
        }

        const char *srcNodeAlias;
        const char *destNodeAlias;

        /* Reduce traverse operation and filters into a single 
         * traverse operation. */
        OpNode *filteredTraverseOp;
        Traverse *t;
        CondTraverse *ct;
        AlgebraicExpression *ae;
        Graph *g;

        switch(traverseOp->operation->type) {
            case OPType_TRAVERSE:
                t = (Traverse*)(traverseOp->operation);
                ae = t->algebraic_expression;
                g = t->graph;
                break;
            case OPType_CONDITIONAL_TRAVERSE:
                ct = (CondTraverse*)(traverseOp->operation);
                ae = ct->algebraic_expression;
                g = ct->graph;
                break;
            default:
                assert(0);
        }

        srcNodeAlias = QueryGraph_GetNodeAlias(plan->graph, *ae->src_node);
        destNodeAlias =  QueryGraph_GetNodeAlias(plan->graph, *ae->dest_node);
        FT_FilterNode *leftHand = NULL;
        FT_FilterNode *rightHand = NULL;
        _groupFilters(filters, srcNodeAlias, destNodeAlias, &leftHand, &rightHand);
        _applyLeftHandFilters(leftHand, ae, g);
        _applyRightHandFilters(rightHand, ae, g);

        // Remove reduced filter operations from execution plan.
        for(int i = 0; i < Vector_Size(filterOps); i++) {
            OpNode* filterOp;
            Vector_Get(filterOps, i, &filterOp);
            ExecutionPlan_RemoveOp(filterOp);
            OpNode_Free(filterOp);
        }
        Vector_Clear(filterOps);        
    }

    Vector_Free(traverseOps);
}
