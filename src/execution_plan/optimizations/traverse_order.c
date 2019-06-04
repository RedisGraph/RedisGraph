/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./traverse_order.h"
#include "../../util/arr.h"
#include "../../util/vector.h"
#include "../../util/rmalloc.h"
#include <assert.h>

#define T 1     // Transpose penalty.
#define F 4 * T // Filter score.

typedef AlgebraicExpression** Arrangement;

// Create a new arrangement.
Arrangement Arrangement_New(uint size) {
    return rm_malloc(sizeof(AlgebraicExpression*) * size);
}

// Clone arrangement.
Arrangement Arrangement_Clone(const Arrangement arrangement, uint size) {
    assert(arrangement);
    Arrangement clone = Arrangement_New(size);
    memcpy(clone, arrangement, sizeof(AlgebraicExpression*) * size);
    return clone;
}

void Arrangement_Print(Arrangement arrangement, uint size) {
    printf("Arrangement_Print\n");
    for(uint i = 0; i < size; i++) {
        AlgebraicExpression *exp = arrangement[i];
        printf("%d, src: %s, dest: %s\n", i, exp->src_node->alias, exp->dest_node->alias);
    }
}

// Free arrangement.
void Arrangement_Free(Arrangement arrangement) {
    assert(arrangement);
    rm_free(arrangement);
}

// Computes x!
static uint factorial(uint x) {
    uint res = 1;
    for(int i = 2; i <= x; i++) res *= i;
    return res;
}

/* Function to swap values at two pointers */
static void swap(Arrangement exps, uint i, uint  j) {
    AlgebraicExpression *temp;
    temp = exps[i]; 
    exps[i] = exps[j];
    exps[j] = temp;
}

/* Computes all permutations of set exps. */
void permute(Arrangement set, int l, int r, Arrangement **permutations) {
    int i;
    if (l == r) {
        Arrangement permutation = Arrangement_Clone(set, r+1);
        *permutations = array_append(*permutations, permutation);
    } else {
        for(i = l; i <= r; i++) {
            swap(set, l, i);
            permute(set, l + 1, r, permutations);
            swap(set, l, i);   // backtrack.
        }
    }
}

// Computes all possible permutations of exps.
static Arrangement* permutations(const Arrangement exps, uint exps_count) {
    // Number of permutations of a set S is |S|!.    
    uint permutation_count = factorial(exps_count);
    Arrangement *permutations = array_new(Arrangement, permutation_count);

    // Compute permutations.
    permute(exps, 0, exps_count-1, &permutations);
    assert(array_len(permutations) == permutation_count);

    return permutations;
}

static int penalty_arrangement(Arrangement arrangement, uint exp_count) {
    int penalty = 0;
    AlgebraicExpression *exp;
    
    // Account for first expression transposes.
    exp = arrangement[0];
    for(uint i = 0; i < exp->operand_count; i++) {
        if(arrangement[0]->operands[i].transpose) penalty += T;
    }

    for(uint i = 1; i < exp_count; i++) {
        exp = arrangement[i];
        bool src_resolved = false;
        Node *src = exp->src_node;

        // See if source is already resolved.
        for(int j = i-1; j >= 0; j--) {
            AlgebraicExpression *prev_exp = arrangement[j];
            if(prev_exp->src_node == src || prev_exp->dest_node == src) {
                src_resolved = true;
                break;
            }
        }

        // dest_node must be resolved as we're working with valid arrangemet.
        if(src_resolved) {
            // Count how many transposes are performed.
            for(uint k = 0; k < exp->operand_count; k++) {
                if(exp->operands[k].transpose) penalty += T;
            }
        } else {
            // Count how many transposes we require to perform.
            for(uint k = 0; k < exp->operand_count; k++) {
                if(!exp->operands[k].transpose) penalty += T;
            }
        }
    }

    return penalty;
}

int score_arrangement(Arrangement arrangement, uint exp_count) {
    int score = 0;
    int penalty = penalty_arrangement(arrangement, exp_count);
    score -= penalty;
    return score;
}

/* A valid arrangement of expressions is one in which the ith expression
 * source or destination nodes appear in a previous expression k where k < i. */
static bool valid_arrangement(const Arrangement arrangement, uint exps_count) {
    for(int i = 1; i < exps_count; i++) {        
        AlgebraicExpression *exp = arrangement[i];
        Node *src = exp->src_node;
        Node *dest = exp->dest_node;
        int j = i-1;

        // Scan previous expressions.
        for(; j >= 0; j--) {
            AlgebraicExpression *prev_exp = arrangement[j];
            if(prev_exp->src_node == src ||
                prev_exp->dest_node == src ||
                prev_exp->src_node == dest ||
                prev_exp->dest_node == dest) break;
        }
        /* Nither src or dest nodes are mentioned in previous expressions
         * as such the arrangement is invalid. */
        if(j < 0) return false;
    }
    return true;
}

/* Given a set of algebraic expressions representing a graph traversal 
 * we pick the order in which the expressions will be evaluated 
 * taking into account filters and transposes. 
 * exps will reordered. */
void orderExpressions(AlgebraicExpression **exps, uint exps_count) {
    assert(exps);

    // Single expression, return quickly.
    if(exps_count == 1) return;

    // Compute all possible permutations of algebraic expressions.
    Arrangement *arrangements = permutations(exps, exps_count);
    uint arrangement_count = array_len(arrangements);

    // Remove invalid arrangements.
    Arrangement *valid_arrangements = array_new(Arrangement, arrangement_count);
    for(int i = 0; i < arrangement_count; i++) {
        if(valid_arrangement(arrangements[i], exps_count)) {
            valid_arrangements = array_append(valid_arrangements, arrangements[i]);
        }
    }
    arrangement_count = array_len(valid_arrangements);

    /* Score each arrangement, 
     * keep track after arrangement with highest score. */
    int max_score = INT_MIN;
    Arrangement top_arrangement;
    for(uint i = 0; i < arrangement_count; i++) {
        Arrangement arrangement = valid_arrangements[i];
        int score = score_arrangement(arrangement, exps_count);
        if(max_score < score) {
            max_score = score;
            top_arrangement = arrangement;
        }
    }

    // Update input.
    for(uint i = 0; i < exps_count; i++) exps[i] = top_arrangement[i];

    // Clean up.
    for(uint i = 0; i < arrangement_count; i++) Arrangement_Free(arrangements[i]);
    array_free(arrangements);
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
