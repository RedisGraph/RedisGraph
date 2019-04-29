/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./traverse_order.h"
#include "../../util/vector.h"

/* Given a set of algebraic expressions and the entire filter tree,
 * suggest traversal entry point to be either the first expression or the
 * last expression, in the future we'll want to be able to begin traversal from 
 * any expression. */
TRAVERSE_ORDER determineTraverseOrder(const FT_FilterNode *filterTree,
                                      AlgebraicExpression **exps,
                                      size_t expCount) {

    if(expCount == 1 || !filterTree) {
        return TRAVERSE_ORDER_FIRST;
    }
    
    bool firstExpLabeled = false;
    bool lastExpLabeled = false;
    AlgebraicExpression *exp;
    TRAVERSE_ORDER order = TRAVERSE_ORDER_FIRST;
    
    uint *modifies = FilterTree_CollectModified(filterTree);
    uint modifiesCount = array_len(modifies);

    // See if there's a filter applied to the first expression.
    exp = exps[0];
    firstExpLabeled = exp->src_node->label || exp->dest_node->label;
    uint dest_idx = exp->dest_node_idx;
    uint src_idx = exp->src_node_idx;

    for(uint i = 0; i < modifiesCount; i++) {
        uint id = modifies[i];
        if (id == src_idx || id == dest_idx) {
            order = TRAVERSE_ORDER_FIRST;
            goto cleanup;
        }
    }

    // See if there's a filter applied to the last expression.
    exp = exps[expCount-1];
    lastExpLabeled = exp->src_node->label || exp->dest_node->label;
    dest_idx = exp->dest_node_idx;
    src_idx = exp->src_node_idx;

    for(uint i = 0; i < modifiesCount; i++) {
        uint id = modifies[i];
        if (id == src_idx || id == dest_idx) {
            order = TRAVERSE_ORDER_LAST;
            goto cleanup;
        }
    }

    /* If we're here, there are no filters on either the first or the last
     * expression. The next-best criteria for choosing traversal order is
     * to prefer an expression in which the source or destination has a label,
     * as label scans are significantly faster than scanning all nodes.
     * If both expressions use labels (or neither do), we'll default
     * to TRAVERSE_ORDER_FIRST. */
    if(!firstExpLabeled && lastExpLabeled) order = TRAVERSE_ORDER_LAST;

cleanup:
    array_free(modifies);
    return order;
}
