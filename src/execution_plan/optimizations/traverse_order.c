/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./traverse_order.h"
#include "../../rmutil/vector.h"

/* Given a set of algebraic expressions and the entire filter tree,
 * suggest traversal entry point to be either the first expression or the
 * last expression, in the future we'll want to be able to begin traversal from 
 * any expression. */
TRAVERSE_ORDER determineTraverseOrder(const QueryGraph *qg,
                                      const FT_FilterNode *filterTree,
                                      AlgebraicExpression **exps,
                                      size_t expCount) {

    if(expCount == 1 || !filterTree) {
        return TRAVERSE_ORDER_LAST;
    }
    
    char *destAlias;
    char *srcAlias;
    AlgebraicExpression *exp;

    TRAVERSE_ORDER order = TRAVERSE_ORDER_FIRST;
    Vector *aliases = FilterTree_CollectAliases(filterTree);
    size_t aliasesCount = Vector_Size(aliases);

    // See if there's a filter applied to the first expression.
    exp = exps[0];
    destAlias = QueryGraph_GetNodeAlias(qg, *exp->dest_node);
    srcAlias = QueryGraph_GetNodeAlias(qg, *exp->src_node);

    for(int i = 0; i < aliasesCount; i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        if(strcmp(alias, srcAlias) == 0 || strcmp(alias, destAlias) == 0) {
            order = TRAVERSE_ORDER_FIRST;
            goto cleanup;
        }
    }

    // See if there's a filter applied to the last expression.
    exp = exps[expCount-1];
    destAlias = QueryGraph_GetNodeAlias(qg, *exp->dest_node);
    srcAlias = QueryGraph_GetNodeAlias(qg, *exp->src_node);

    for(int i = 0; i < aliasesCount; i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        if(strcmp(alias, srcAlias) == 0 || strcmp(alias, destAlias) == 0) {
            order = TRAVERSE_ORDER_LAST;
            goto cleanup;
        }
    }

cleanup:
    Vector_Free(aliases);
    return order;
}
