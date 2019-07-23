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
#define L 2 * T // Label score.

typedef AlgebraicExpression **Arrangement;

// Create a new arrangement.
Arrangement Arrangement_New(uint size) {
	return rm_malloc(sizeof(AlgebraicExpression *) * size);
}

// Clone arrangement.
Arrangement Arrangement_Clone(const Arrangement arrangement, uint size) {
	assert(arrangement);
	Arrangement clone = Arrangement_New(size);
	memcpy(clone, arrangement, sizeof(AlgebraicExpression *) * size);
	return clone;
}

// Print arrangement.
void Arrangement_Print(Arrangement arrangement, uint size) {
	printf("Arrangement_Print\n");
	for(uint i = 0; i < size; i++) {
		AlgebraicExpression *exp = arrangement[i];
		printf("%d, src: %s, dest: %s\n", i, exp->src_node->alias,
			   exp->dest_node->alias);
	}
}

// Free arrangement.
void Arrangement_Free(Arrangement arrangement) {
	assert(arrangement);
	rm_free(arrangement);
}

// Computes x!
static unsigned long factorial(uint x) {
	unsigned long res = 1;
	for(int i = 2; i <= x; i++) res *= i;
	return res;
}

// Function to swap values at two pointers.
static void swap(Arrangement exps, uint i, uint  j) {
	AlgebraicExpression *temp;
	temp = exps[i];
	exps[i] = exps[j];
	exps[j] = temp;
}

// Computes all permutations of set exps.
void permute(Arrangement set, int l, int r, Arrangement **permutations) {
	int i;
	if(l == r) {
		Arrangement permutation = Arrangement_Clone(set, r + 1);
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
static Arrangement *permutations(const Arrangement exps, uint exps_count) {
	// Number of permutations of a set S is |S|!.
	unsigned long permutation_count = factorial(exps_count);
	Arrangement *permutations = array_new(Arrangement, permutation_count);

	// Compute permutations.
	permute(exps, 0, exps_count - 1, &permutations);
	assert(array_len(permutations) == permutation_count);

	return permutations;
}

/* A valid arrangement of expressions is one in which the ith expression
 * source or destination nodes appear in a previous expression k where k < i. */
static bool valid_arrangement(const Arrangement arrangement, uint exps_count) {
	AlgebraicExpression *exp = arrangement[0];
	/* A 1 hop traversals where either the source node
	 * or destination node is labeled, can't be the opening expression
	 * in an arrangement.
	 * Consider: MATCH (a:L0)-[:R*]->(b:L1)
	 * [L0] * [R] * [L1] but because R is a variable length traversal
	 * we're dealing with 3 different expressions:
	 * exp0: [L0]
	 * exp1: [R]
	 * exp2: [L1]
	 * the arrangement where [R] is the first expression:
	 * exp0: [R]
	 * exp1: [L0]
	 * exp2: [L1]
	 * Isn't valid, as currently the first expression is converted
	 * into a scan operation. */
	if((exp->src_node->label || exp->dest_node->label) &&
	   exp->edge &&
	   exp->operand_count == 1) return false;

	for(int i = 1; i < exps_count; i++) {
		exp = arrangement[i];
		Node *src = exp->src_node;
		Node *dest = exp->dest_node;
		int j = i - 1;

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
		for(int j = i - 1; j >= 0; j--) {
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

static int reward_arrangement(Arrangement arrangement, uint exp_count,
							  const FT_FilterNode *filters) {
	// Arrangement_Print(arrangement, exp_count);
	int reward = 0;
	rax *filtered_entities = FilterTree_CollectAliases(filters);

	// A bit naive at the moment.
	for(uint i = 0; i < exp_count; i++) {
		AlgebraicExpression *exp = arrangement[i];
		char *src_alias = exp->src_node->alias;
		char *dest_alias = exp->dest_node->alias;

		if(raxFind(filtered_entities, (unsigned char *)src_alias,
				   strlen(src_alias)) != raxNotFound) {
			reward += F * (exp_count - i);
			raxRemove(filtered_entities, (unsigned char *)src_alias, strlen(src_alias),
					  NULL);
		}
		if(raxFind(filtered_entities, (unsigned char *)dest_alias,
				   strlen(dest_alias)) != raxNotFound) {
			reward += F * (exp_count - i);
			raxRemove(filtered_entities, (unsigned char *)dest_alias, strlen(dest_alias),
					  NULL);
		}
		if(exp->src_node->label) reward += L * (exp_count - i);
	}

	// printf("reward: %d\n", reward);
	raxFree(filtered_entities);
	return reward;
}

static int score_arrangement(Arrangement arrangement, uint exp_count,
							 const FT_FilterNode *filters) {
	int score = 0;
	int penalty = penalty_arrangement(arrangement, exp_count);
	int reward = reward_arrangement(arrangement, exp_count, filters);
	score -= penalty;
	score += reward;
	return score;
}

/* Given a set of algebraic expressions representing a graph traversal
 * we pick the order in which the expressions will be evaluated
 * taking into account filters and transposes.
 * exps will reordered. */
void orderExpressions(AlgebraicExpression **exps, uint exps_count,
					  const FT_FilterNode *filters) {
	assert(exps && exps_count > 0);

	// Single expression, return quickly.
	if(exps_count == 1) return;

	// Compute all possible permutations of algebraic expressions.
	Arrangement *arrangements = permutations(exps, exps_count);
	uint arrangement_count = array_len(arrangements);
	if(arrangement_count == 1) return;

	// Remove invalid arrangements.
	Arrangement *valid_arrangements = array_new(Arrangement, arrangement_count);
	for(int i = 0; i < arrangement_count; i++) {
		if(valid_arrangement(arrangements[i], exps_count)) {
			valid_arrangements = array_append(valid_arrangements, arrangements[i]);
		}
	}
	arrangement_count = array_len(valid_arrangements);
	assert(arrangement_count > 0);

	/* Score each arrangement,
	 * keep track after arrangement with highest score. */
	int max_score = INT_MIN;
	Arrangement top_arrangement = valid_arrangements[0];

	for(uint i = 0; i < arrangement_count; i++) {
		Arrangement arrangement = valid_arrangements[i];
		int score = score_arrangement(arrangement, exps_count, filters);
		// printf("score: %d\n", score);
		// Arrangement_Print(arrangement, exps_count);
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
