#include "reduce_filters.h"
#include "../ops/op_filter.h"
#include "../../filter_tree/filter_tree.h"
#include "../../parser/grammar.h"

void _reduceFilter(OpNode *op) {
    OpNode *parent = op;
    Filter *filter = (Filter*)parent->operation;
    FT_FilterNode *tree = filter->filterTree;
    OpNode *child = NULL;

    /* Filter operation is promised to have only one child. */
    while(parent->childCount == 1) {
        child = parent->children[0];
        if(child->operation->type != OPType_FILTER) break;

        Filter *childFilter = (Filter*)child->operation;

        /* Create a new root for the tree, merge trees using an AND. */
        FT_FilterNode *root = CreateCondFilterNode(AND);
        AppendLeftChild(root, tree);
        AppendLeftChild(root, childFilter->filterTree);
        tree = root;

        // Proceed.
        parent = child;
    }

    // Did we performed a reduction?
    if(filter->filterTree != tree) {
        // Remove intermidate filter ops.
        OpNode *intermidateChild = child->parent;
        while(intermidateChild != op) {
            parent = intermidateChild->parent;
            OpNode_Free(intermidateChild);
            intermidateChild = parent;
        }
        
        /* child is the first operation we encountered which is not of type filter.
         * update child parent to reduced filter op
         * update reduced filter op child. */
        child->parent = op;
        op->children[0] = child;
    }
}

void _reduceFilters(OpNode *op) {
    if(op == NULL) return;
    
    if(op->operation->type == OPType_FILTER) {
        _reduceFilter(op);
    }

    for(int i = 0; i < op->childCount; i++) {
        _reduceFilters(op->children[i]);
    }
}

void reduceFilters(ExecutionPlan *plan) {
    return _reduceFilters(plan->root);
}
