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
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"
#include <assert.h>

void singleMatrixFilter(AlgebraicExpression *ae, Graph *g, Vector *filters);

bool _filter_row_select_op(const GrB_Index i, const GrB_Index j, const GrB_Index nrows,
                    const GrB_Index ncols, const void *x, const void *k) {
    GrB_Vector v = (GrB_Vector)k;
    // keep entry A(i,j), unless row i or column j are marked.
    bool mark = false;
    GrB_Vector_extractElement_BOOL(&mark, v, i);
    return mark;
}

/* Separates filters into two groups:
 * 1. filters applied to the first term rows (left hand)
 * 2. filters applied to the last term columns (right hand) */
void _groupFilters(AlgebraicExpression *ae,
                   Vector *filters,
                   const char *srcNodeAlias,
                   const char *destNodeAlias,
                   FT_FilterNode **leftHand,
                   FT_FilterNode **rightHand) {
    
    /* Depending on rather or not the algebraic expression been transposed,
     * source and destination might have been swapped, if expression been transposed
     * then source is represented by the matrix columns (right hand) and destination
     * is represented by the matrix rows (left hand), otherwise, expression wasn't 
     * transposed, then source is represented by the matrix rows (left hand) and 
     * destination is represented by the matrix columns (right hand). */

    for(int i = 0; i < Vector_Size(filters); i++) {
        char *alias;
        FT_FilterNode *filter;
        Vector_Get(filters, i , &filter);
        
        // Expecting only one alias.
        Vector *filteredEntities = FilterTree_CollectAliases(filter);

        Vector_Get(filteredEntities, 0, &alias);
        Vector_Free(filteredEntities);

        // Determin if filter aplies to source node or dest node.
        FT_FilterNode **hand;
        if(strcmp(alias, srcNodeAlias) == 0) hand = leftHand;
        else hand = rightHand;

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

// GrB_Matrix _filtersToMatrix(FT_FilterNode *filter, AlgebraicExpressionOperand *term, bool transpose, const Graph *g, Node **filteredEntity) {
//     GrB_Matrix M = term->operand;
    
//     // Determin which columns are active.
//     GrB_Descriptor desc;
//     GrB_Descriptor_new(&desc);

//     /* Matrix reduction is performed on rows 
//      * while we're interested in reducing columns
//      * and so if the term is transposed then we're all set
//      * otherwise we have to transpose. */
//     if(transpose) {
//         GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
//     }
//     GrB_Index nrows;
//     GrB_Matrix_nrows(&nrows, M);

//     GrB_Vector activeEntries;
//     GrB_Vector_new(&activeEntries, GrB_BOOL, nrows);
//     GrB_Matrix_reduce_BinaryOp(activeEntries, NULL, NULL, GrB_LOR, M, desc);
//     GrB_Descriptor_free(&desc);

//     GrB_Index nvals;
//     GrB_Vector_nvals(&nvals, activeEntries);

//     // Create filter matrix.    
//     GrB_Matrix filterMatrix;
//     GrB_Matrix_new(&filterMatrix, GrB_BOOL, nrows, nrows);

//     // Scan active entries, apply filter to each entry.
//     GrB_Index entryIdx;
//     Node *originalNode = *filteredEntity;
//     TuplesIter *iter = TuplesIter_new((GrB_Matrix)activeEntries);
//     while(TuplesIter_next(iter, &entryIdx, NULL) != TuplesIter_DEPLETED) {
//         Node *node = Graph_GetNode(g, entryIdx);
//         *filteredEntity = node;
//         if(FilterTree_applyFilters(filter)) {
//             GrB_Matrix_setElement_BOOL(filterMatrix, true, entryIdx, entryIdx);
//         }
//     }

//     TuplesIter_free(iter);
//     GrB_Vector_free(&activeEntries);
    
//     // Restore filtered entity node.
//     *filteredEntity = originalNode;
    
//     return filterMatrix;
// }

/* Filter last algebraic expression term columns by applying filter */
void _applyRightHandFilters(FT_FilterNode *filterTree, AlgebraicExpression *ae, const Graph *g) {
    /* Filter the rightmost matrix in given expression.
     * Steps:    
     * 1. Scans through matrix active columns
     * 2. Apply filter tree to each active column id
     * 3. Remove columns which did not pass filter. */

    if(!filterTree) return;
    // Create filtered matrix, as we can't modified original term.
    GrB_Matrix filteredMatrix;
    GrB_Matrix_new(&filteredMatrix, GrB_BOOL, g->node_count, g->node_count);

    // Get the last term.
    AlgebraicExpressionOperand *term = &(ae->operands[ae->operand_count-1]);
    GrB_Matrix M = term->operand;

    // Save original node to be restored when we're done.
    Node **filteredNode;
    if(AlgebraicExpression_IsTranspose(ae)) filteredNode = ae->src_node;
    else filteredNode = ae->dest_node;
    Node *originalNode = *filteredNode;

    GrB_Vector col = NULL;
    GrB_Vector_new(&col, GrB_BOOL, g->node_count);

    // Inspect each column.
    for(int colIdx = 0; colIdx < g->node_count; colIdx++) {
        GrB_Col_extract(col, NULL, NULL, M, GrB_ALL, g->node_count, colIdx, NULL);
        
        // See if current columns has atleast one none zero.
        GrB_Index nvals;
        GrB_Vector_nvals(&nvals, col);
        if(nvals == 0) continue;

        Node *n = Graph_GetNode(g, colIdx);
        *filteredNode = n;
        if(FilterTree_applyFilters(filterTree)) {
            GrB_Col_assign(filteredMatrix, NULL, NULL, col, GrB_ALL, g->node_count, colIdx, NULL);
        }
    }

    // Restore query graph node.
    *filteredNode = originalNode;

    // Update expression.
    ae->operands[ae->operand_count-1].operand = filteredMatrix;
    ae->operands[ae->operand_count-1].free = true;

    // Clean up
    GrB_Vector_free(&col);
}

void _applyLeftHandFilters(FT_FilterNode *filterTree, AlgebraicExpression *ae, const Graph *g) {
    /* Filter the leftmost matrix in given expression.
     * Steps:    
     * 1. Find out which rows are 'active', i.e. have at least one NNZ in them.
     * 2. Apply filter tree to each active row id, to get a vector of rows which 
     *    we're interested in.
     * 3. Remove entire rows from matrix. */

    if(!filterTree) return;
    GrB_Vector activeRows; // activeRows[i] = 1 if SUM(M[i:]) > 0.
    GrB_Vector filteredRows; // filteredRows[i] = 1 if activeRows[i] AND Node i pass filter.

    /* Find out which rows have at least one none zero value in them. */
    GrB_Vector_new(&activeRows, GrB_BOOL, g->node_count);
    GrB_Vector_new(&filteredRows, GrB_BOOL, g->node_count);
    GrB_Matrix_reduce_BinaryOp(activeRows, NULL, NULL, GrB_LOR, ae->operands[0].operand, NULL);
    
    // Save original node to be restored when we're done.
    Node **filteredNode;
    if(AlgebraicExpression_IsTranspose(ae)) filteredNode = ae->dest_node;
    else filteredNode = ae->src_node;
    Node *originalNode = *filteredNode;

    GrB_Index rowIdx;
    TuplesIter *it = TuplesIter_new((GrB_Matrix)activeRows);

    /* Apply filter to every active row. */
    while(TuplesIter_next(it, &rowIdx, NULL) != TuplesIter_DEPLETED) {
        Node *n = Graph_GetNode(g, rowIdx);
        // Update node and apply filter.
        *filteredNode = n;
        if(FilterTree_applyFilters(filterTree)) {
            GrB_Vector_setElement_BOOL(filteredRows, true, rowIdx);
        }
    }

    // Restore query graph node.
    *filteredNode = originalNode;

    // Remove rows which did not pass filter.    
    GxB_SelectOp select;
    GxB_SelectOp_new(&select, &_filter_row_select_op, NULL);

    GrB_Matrix filteredMatrix;
    GrB_Matrix_new(&filteredMatrix, GrB_BOOL, g->node_count, g->node_count);
    GxB_Matrix_select(filteredMatrix, NULL, NULL, select, ae->operands[0].operand, filteredRows, NULL);

    ae->operands[0].operand = filteredMatrix;
    ae->operands[0].free = true;

    // Clean up.
    TuplesIter_free(it);
    GxB_SelectOp_free(&select);
    GrB_Vector_free(&activeRows);
    GrB_Vector_free(&filteredRows);
}

/* Populate filterOps vector with execution plan filter operations
 * applied to given traverseOp. */
void _locateTraverseFilters(const OpBase *traverseOp, Vector *filterOps) {
    assert(traverseOp->type == OPType_TRAVERSE ||
           traverseOp->type == OPType_CONDITIONAL_TRAVERSE);

    /* Scanning execution plan from traverseOp "downwards" using parent pointer
     * until we hit a none filter operation. */
    OpBase *current = traverseOp->parent;
    while(current->type == OPType_FILTER) {
        /* We're only interested in filters which apply to a single entity,
         * Extract modified aliases from filter tree. */
        Filter *filterOp = (Filter*)current;
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
void _locateTraverseOp(OpBase *root, Vector *traverseOps) {

    // Is this a traverse operation?
    if(root->type == OPType_TRAVERSE ||
       root->type == OPType_CONDITIONAL_TRAVERSE) {
           Vector_Push(traverseOps, root);
    }

    // Continue scanning.
    for(int i = 0; i < root->childCount; i++) {        
        _locateTraverseOp(root->children[i], traverseOps);
    }
}

void translateFilters(ExecutionPlan *plan) {    
    // Get a list of all traverse operations within execution plan.
    Vector *traverseOps = NewVector(OpBase*, 0);
    _locateTraverseOp(plan->root, traverseOps);

    /* For each traverse op, locate filters applied 
     * to their modified entities. */
    OpBase *traverseOp;
    Vector *filterOps = NewVector(OpBase*, 0);

    while(Vector_Pop(traverseOps, &traverseOp)) {
        _locateTraverseFilters(traverseOp, filterOps);
        if(Vector_Size(filterOps) == 0) continue;

        // Extract actual filter trees.
        Vector *filters = NewVector(FT_FilterNode*, Vector_Size(filterOps));
        for(int i = 0; i < Vector_Size(filterOps); i++) {            
            OpBase *opFilter;
            Vector_Get(filterOps, i, &opFilter);
            Filter *f = (Filter *)opFilter;
            Vector_Push(filters, f->filterTree);
        }

        const char *srcNodeAlias;
        const char *destNodeAlias;

        /* Reduce traverse operation and filters into a single 
         * traverse operation. */
        OpBase *filteredTraverseOp;
        Traverse *t;
        CondTraverse *ct;
        AlgebraicExpression *ae;
        Graph *g;

        switch(traverseOp->type) {
            case OPType_TRAVERSE:
                t = (Traverse*)(traverseOp);
                ae = t->algebraic_expression;
                g = t->graph;
                break;
            case OPType_CONDITIONAL_TRAVERSE:
                ct = (CondTraverse*)(traverseOp);
                ae = ct->algebraic_expression;
                g = ct->graph;
                break;
            default:
                assert(0);
        }

        _locateTraverseFilters(traverseOp, filterOps);

        // No filters.
        if(Vector_Size(filterOps) == 0) continue;

        // Extract actual filter trees.
        Vector *filters = NewVector(FT_FilterNode*, Vector_Size(filterOps));
        for(int i = 0; i < Vector_Size(filterOps); i++) {            
            OpNode *opFilter;
            Vector_Get(filterOps, i, &opFilter);
            Filter *f = (Filter *)opFilter->operation;
            Vector_Push(filters, f->filterTree);
        }

        // Don't care, as no multiplications will be performed.
        if(ae->operand_count == 1) {
            singleMatrixFilter(ae, g, filters);
        } else {
            const char *srcNodeAlias = NULL;
            const char *destNodeAlias = NULL;
            if(AlgebraicExpression_IsTranspose(ae)) {
                destNodeAlias = QueryGraph_GetNodeAlias(plan->graph, *ae->src_node);
                srcNodeAlias = QueryGraph_GetNodeAlias(plan->graph, *ae->dest_node);
            } else {
                srcNodeAlias = QueryGraph_GetNodeAlias(plan->graph, *ae->src_node);
                destNodeAlias = QueryGraph_GetNodeAlias(plan->graph, *ae->dest_node);
            }

            FT_FilterNode *leftHand = NULL;
            FT_FilterNode *rightHand = NULL;
            _groupFilters(ae, filters, srcNodeAlias, destNodeAlias, &leftHand, &rightHand);
            _applyRightHandFilters(rightHand, ae, g);
            _applyLeftHandFilters(leftHand, ae, g);
        }

        // Remove reduced filter operations from execution plan.
        for(int i = 0; i < Vector_Size(filterOps); i++) {
            OpBase* filterOp;
            Vector_Get(filterOps, i, &filterOp);
            ExecutionPlan_RemoveOp(filterOp);
            OpBase_Free(filterOp);
        }
        Vector_Clear(filterOps);
        Vector_Free(filters);
    }

    // Cleanup
    Vector_Free(filterOps);
    Vector_Free(traverseOps);
}

typedef struct {
    Graph *g;
    AlgebraicExpression *ae;
    FT_FilterNode *filter;
    bool transposed;
} MatrixFilter;

bool _single_matrix_filter_select_op(const GrB_Index i, const GrB_Index j, const GrB_Index nrows, const GrB_Index ncols, const void *x, const void *k) {
    MatrixFilter *mf = (MatrixFilter*)k;
    AlgebraicExpression *ae = mf->ae;
    FT_FilterNode *filter = mf->filter;
    Graph *g = mf->g;
    bool transposed = mf->transposed;

    Node *srcNode = Graph_GetNode(g, i);
    Node *destNode = Graph_GetNode(g, j);
    if(transposed) {
        *ae->src_node = destNode;
        *ae->dest_node = srcNode;
    } else {
        *ae->src_node = srcNode;
        *ae->dest_node = destNode;
    }
    return FilterTree_applyFilters(filter);
}

void singleMatrixFilter(AlgebraicExpression *ae, Graph *g, Vector *filters) {
    // Combine filters.
    FT_FilterNode *root = NULL;
    for(int i = 0; i < Vector_Size(filters); i++) {            
        FT_FilterNode *filterTree;
        Vector_Get(filters, i, &filterTree);
        if(root == NULL) {
            root = filterTree;
        } else {
            FT_FilterNode *and = CreateCondFilterNode(AND);
            AppendLeftChild(and, filterTree);
            AppendRightChild(and, root);
            root = and;
        }
    }

    MatrixFilter mf;
    mf.g = g;
    mf.ae = ae;
    mf.filter = root;
    mf.transposed = AlgebraicExpression_IsTranspose(ae);

    // Backup.
    Node *srcOrg = *ae->src_node;
    Node *destOrg = *ae->dest_node;

    GxB_SelectOp select;
    GxB_SelectOp_new(&select, &_single_matrix_filter_select_op, NULL);
    
    GrB_Matrix filteredMatrix;
    GrB_Matrix_new(&filteredMatrix, GrB_BOOL, g->node_count, g->node_count);
    GxB_Matrix_select(filteredMatrix, NULL, NULL, select, ae->operands[0].operand, &mf, NULL);

    // Restore
    *ae->src_node = srcOrg;
    *ae->dest_node = destOrg;

    ae->operands[0].operand = filteredMatrix;
    ae->operands[0].free = true;
}
