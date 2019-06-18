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

    if(expCount == 1) {
        return TRAVERSE_ORDER_FIRST;
    }
    AlgebraicExpression *firstExp = exps[0];
    AlgebraicExpression *lastExp = exps[expCount-1];

    if (firstExp->operand_count == 1 && firstExp->edgeLength == NULL) return TRAVERSE_ORDER_FIRST;
    if (lastExp->operand_count == 1 && lastExp->edgeLength == NULL) return TRAVERSE_ORDER_LAST;

    bool firstExpLabeled = firstExp->src_node->label || firstExp->dest_node->label;
    bool lastExpLabeled = lastExp->src_node->label || lastExp->dest_node->label;

    /* If we have no filters, favor a starting expression in which
     * the source or destination is labeled. */
    if(filterTree == NULL) {
        if(!firstExpLabeled && lastExpLabeled) {
            return TRAVERSE_ORDER_LAST;
        } else {
            return TRAVERSE_ORDER_FIRST;
        }
    }

    char *destAlias;
    char *srcAlias;
    TRAVERSE_ORDER order = TRAVERSE_ORDER_FIRST;
    
    Vector *aliases = FilterTree_CollectAliases(filterTree);
    size_t aliasesCount = Vector_Size(aliases);

    // See if there's a filter applied to the first expression.
    destAlias = firstExp->dest_node->alias;
    srcAlias = firstExp->src_node->alias;

    for(int i = 0; i < aliasesCount; i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        if(strcmp(alias, srcAlias) == 0 || strcmp(alias, destAlias) == 0) {
            order = TRAVERSE_ORDER_FIRST;
            goto cleanup;
        }
    }

    // See if there's a filter applied to the last expression.
    destAlias = lastExp->dest_node->alias;
    srcAlias = lastExp->src_node->alias;

    for(int i = 0; i < aliasesCount; i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        if(strcmp(alias, srcAlias) == 0 || strcmp(alias, destAlias) == 0) {
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
    for(int i = 0; i < aliasesCount; i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        free(alias);
    }
    Vector_Free(aliases);
    return order;
}
