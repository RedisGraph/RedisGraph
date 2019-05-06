/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_filters.h"
#include "../ops/op_filter.h"
#include "../../filter_tree/filter_tree.h"

void _reduceFilter(OpBase *op) {
    OpBase *parent = op;
    Filter *filter = (Filter*)parent;
    FT_FilterNode *tree = filter->filterTree;
    OpBase *child = NULL;

    /* Filter operation is promised to have only one child. */
    while(parent->childCount == 1) {
        child = parent->children[0];
        if(child->type != OPType_FILTER) break;

        Filter *childFilter = (Filter*)child;

        /* Create a new root for the tree, merge trees using an AND. */
        FT_FilterNode *root = CreateCondFilterNode(OP_AND);
        AppendLeftChild(root, tree);
        AppendRightChild(root, childFilter->filterTree);
        tree = root;

        // Proceed.
        parent = child;
    }

    // Did we performed a reduction?
    if(filter->filterTree != tree) {
        filter->filterTree = tree;
        // Remove intermidate filter ops.
        OpBase *intermidateChild = child->parent;
        while(intermidateChild != op) {
            parent = intermidateChild->parent;
            // Remove the filter tree pointer from the intermediate op, as it should not be freed
            ((Filter*)intermidateChild)->filterTree = NULL;
            OpBase_Free(intermidateChild);
            intermidateChild = parent;
        }
        
        /* child is the first operation we encountered which is not of type filter.
         * update child parent to reduced filter op
         * update reduced filter op child. */
        child->parent = op;
        op->children[0] = child;
    }
}

void _reduceFilters(OpBase *op) {
    if(op == NULL) return;
    
    if(op->type == OPType_FILTER) {
        _reduceFilter(op);
    }

    for(int i = 0; i < op->childCount; i++) {
        _reduceFilters(op->children[i]);
    }
}

void reduceFilters(ExecutionPlanSegment *plan) {
    return _reduceFilters(plan->root);
}
