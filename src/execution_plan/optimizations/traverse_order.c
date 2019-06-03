/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./traverse_order.h"
#include "../../util/vector.h"
#include "../../util/arr.h"
#include <assert.h>

#define T 1     // Transpose penalty.
#define F 4 * T // Filter score.

typedef AlgebraicExpression** Arrangement;

// Computes x!
static uint factorial(uint x) {
    uint res = 1;
    for(int i = 2; i <= x; i++) res *= i;
    return res;
}

/* A valid arrangement of expressions is one in which the ith expression
 * source or destination nodes appear in a previous expression k where k < i. */
static bool valid_arrangement(const Arrangement arrangement, uint exps_count) {
    for(int i = 1; i < exps_count; i++) {
        AlgebraicExpression *exp = arrangement[i];
        Node *src = exp->src_node;
        Node *dest = exp->dest_node;
        int j = i;

        // Scan previous expressions.
        for(; j >= 0; j--) {
            AlgebraicExpression *prev_exp = arrangement[j];
            if(prev_exp->src_node == src || prev_exp->dest_node == dest) break;

            /* Nither src or dest nodes are mentioned in previous expressions
             * as such the arrangement is invalid. */
            if(j < 0) return false;
        }
    }
    return true;
}

/* Function to swap values at two pointers */
static void swap(Arrangement exps, uint i, uint  j) {
    AlgebraicExpression *temp;
    temp = exps[i]; 
    exps[i] = exps[j];
    exps[j] = temp;
}

/* Computes all permutations of set exps. */
void permute(Arrangement set, int l, int r, Arrangement *permutations) {
    int i;
    if (l == r) {
        Arrangement permutation = rm_malloc(sizeof(AlgebraicExpression*) * r);
        memcpy(permutation, set, sizeof(AlgebraicExpression*) * r);
        permutations = array_append(permutations, permutation);
    } else {
        for(i = l; i <= r; i++) {
            swap(set, l, i);
            permute(set, l + 1, r, permutations);
            swap(set, l, i);   // backtrack.
        }
    }
}

// Computes all possible permutations of exps.
static Arrangement* valid_permutations(const Arrangement exps, uint exps_count) {
    // Number of permutations of a set S is |S|!.    
    uint permutation_count = factorial(exps_count);
    Arrangement *permutations = array_new(Arrangement, permutation_count);

    // Compute permutations.
    permute(exps, 0, exps_count, permutations);
    assert(array_len(permutations) == permutation_count);

    // Remove invalid arrangements.
    Arrangement *valid_arrangements = array_new(Arrangement, permutation_count);
    for(int i = 0; i < permutation_count; i++) {
        if(valid_arrangement(permutations[i], exps_count)) {
            valid_arrangements = array_append(valid_arrangements, permutations[i]);
        }
    }

    // Clean up.
    for(int i = 0; i < permutation_count; i++) rm_free(permutations[i]);
    array_free(permutations);

    return valid_arrangements;
}

int score_arrangement(Arrangement arrangement) {
    return 0;
}

void orderExpressions(AlgebraicExpression **exps, uint exps_count) {
    // Compute all possible valid permutations.
    Arrangement *arrangements = valid_permutations(exps, exps_count);
    uint arrangement_count = array_len(arrangements);
    
    /* Score each arrangement, 
     * keep track after arrangement with highest score. */
    int max_score = INT_MIN;
    Arrangement top_arrangement;
    for(uint i = 0; i < arrangement_count; i++) {
        Arrangement arrangement = arrangements[i];
        int score = score_arrangement(arrangement);
        if(max_score < score) {
            max_score = score;
            top_arrangement = arrangement;
        }
    }
}

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
