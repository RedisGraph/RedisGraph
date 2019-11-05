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

#define T 1           // Transpose penalty.
#define F 4 * T       // Filter score.
#define L 2 * T       // Label score.
#define B 8 * F       // Bound variable bonus (TODO choose appropriate val)

typedef AlgebraicExpression **Arrangement;

// Create a new arrangement.
static inline Arrangement Arrangement_New(uint size) {
	return rm_malloc(sizeof(AlgebraicExpression *) * size);
}

// Clone arrangement.
static inline Arrangement Arrangement_Clone(const Arrangement arrangement, uint size) {
	assert(arrangement);
	Arrangement clone = Arrangement_New(size);
	memcpy(clone, arrangement, sizeof(AlgebraicExpression *) * size);
	return clone;
}

// Print arrangement.
static inline void Arrangement_Print(Arrangement arrangement, uint size) {
	printf("Arrangement_Print\n");
	for(uint i = 0; i < size; i++) {
		AlgebraicExpression *exp = arrangement[i];
		printf("%d, src: %s, dest: %s\n", i, exp->src_node->alias, exp->dest_node->alias);
	}
}

// Free arrangement.
static inline void Arrangement_Free(Arrangement arrangement) {
	assert(arrangement);
	rm_free(arrangement);
}

// Computes x!
static inline unsigned long factorial(uint x) {
	unsigned long res = 1;
	for(int i = 2; i <= x; i++) res *= i;
	return res;
}

// Function to swap values at two pointers.
static inline void swap(Arrangement exps, uint i, uint  j) {
	AlgebraicExpression *temp;
	temp = exps[i];
	exps[i] = exps[j];
	exps[j] = temp;
}

// Computes all permutations of set exps.
static inline void permute(Arrangement set, int l, int r, Arrangement **permutations) {
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
		QGNode *src = exp->src_node;
		QGNode *dest = exp->dest_node;
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
		QGNode *src = exp->src_node;

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

static int reward_arrangement(Arrangement arrangement, uint exp_count, rax *filtered_entities,
							  rax *bound_vars) {
	// Arrangement_Print(arrangement, exp_count);
	int reward = 0;

	// A bit naive at the moment.
	for(uint i = 0; i < exp_count; i++) {
		AlgebraicExpression *exp = arrangement[i];

		// Reward bound variables such that any expression with a bound variable
		// will be preferred over any expression without.
		if(raxFind(bound_vars, (unsigned char *)exp->src_node->alias,
				   strlen(exp->src_node->alias)) != raxNotFound) {
			// Give bound source nodes an additional bonus (L) in contrast to bound destinations.
			reward += (B + L) * (exp_count - i);
		}

		if(raxFind(bound_vars, (unsigned char *)exp->dest_node->alias,
				   strlen(exp->dest_node->alias)) != raxNotFound) {
			reward += B * (exp_count - i);
		}

		// Reward filters in expression.
		if(raxFind(filtered_entities, (unsigned char *)exp->src_node->alias,
				   strlen(exp->src_node->alias)) != raxNotFound) {
			reward += F * (exp_count - i);
		}
		if(raxFind(filtered_entities, (unsigned char *)exp->dest_node->alias,
				   strlen(exp->dest_node->alias)) != raxNotFound) {
			reward += F * (exp_count - i);
		}
		if(exp->src_node->label) reward += L * (exp_count - i);
	}

	// printf("reward: %d\n", reward);
	return reward;
}

static int _score_arrangement(Arrangement arrangement, uint exp_count,
							  rax *filtered_entities, rax *bound_vars) {
	int score = 0;
	int penalty = penalty_arrangement(arrangement, exp_count);
	int reward = reward_arrangement(arrangement, exp_count, filtered_entities, bound_vars);
	score -= penalty;
	score += reward;
	return score;
}

// Transpose out-of-order expressions so that each expresson's source is resolved
// in the winning sequence.
static void resolve_winning_sequence(AlgebraicExpression **exps, uint exp_count) {
	for(uint i = 1; i < exp_count; i ++) {
		AlgebraicExpression *exp = exps[i];
		bool src_resolved = false;
		QGNode *src = exp->src_node;

		// See if source is already resolved.
		for(int j = i - 1; j >= 0; j--) {
			AlgebraicExpression *prev_exp = exps[j];
			if(prev_exp->src_node == src || prev_exp->dest_node == src) {
				src_resolved = true;
				break;
			}
		}
		if(!src_resolved) AlgebraicExpression_Transpose(exp);
	}
}

/* Having chosen which algebraic expression will be evaluated first, determine whether
 * it is worthwhile to transpose it and thus swap the source and destination.
 * We'll choose to transpose if the destination is filtered and the source is not, or
 * if neither is filtered, if the destination is labeled and the source is not. */
static void select_entry_point(AlgebraicExpression *ae, rax *filtered_entities, rax *bound_vars) {
	if(ae->operand_count == 1 && ae->src_node == ae->dest_node) return;

	const char *src_name = ae->src_node->alias;
	const char *dest_name = ae->dest_node->alias;

	// Always start at a bound variable if one is present.
	if(raxFind(bound_vars, (unsigned char *)src_name, strlen(src_name)) != raxNotFound) {
		return;
	}

	if(raxFind(bound_vars, (unsigned char *)dest_name, strlen(dest_name)) != raxNotFound) {
		AlgebraicExpression_Transpose(ae);
		return;
	}

	// See if either source or destination nodes are filtered.
	if(raxFind(filtered_entities, (unsigned char *)src_name, strlen(src_name)) != raxNotFound) {
		return; // The source node is filtered, making the current order most appealing.
	}

	bool destFiltered = (raxFind(filtered_entities, (unsigned char *)dest_name,
								 strlen(dest_name)) != raxNotFound);

	/* Prefer filter over label
	 * if no filters are applied prefer labeled entity. */
	bool srcLabeled = ae->src_node->label != NULL;
	bool destLabeled = ae->dest_node->label != NULL;

	/* TODO: when additional statistics are available
	 * do not use label scan if for every node N such that
	 * (N)-[relation]->(T) N is of the same type T, and type of
	 * either source or destination node is T. */
	if(destFiltered) {
		// The destination is filtered and the source is not, transpose.
		AlgebraicExpression_Transpose(ae);
	} else if(srcLabeled) {
		// Neither end is filtered and the source is labeled, making the current order most appealing.
		return;
	} else if(destLabeled) {
		// The destination is labeled and the source is not, transpose.
		AlgebraicExpression_Transpose(ae);
	}
}

/* Given a set of algebraic expressions representing a graph traversal
 * we pick the order in which the expressions will be evaluated
 * taking into account filters and transposes.
 * exps will reordered. */
void orderExpressions(AlgebraicExpression **exps, uint exps_count, const FT_FilterNode *filters,
					  rax *bound_vars) {
	assert(exps && exps_count > 0);

	// We only have one expression, no arrangements to consider.
	if(exps_count == 1) {
		// Our expression is only a scan, no need to select an entry point.
		if(exps[0]->operand_count == 1 && exps[0]->src_node == exps[0]->dest_node) return;

		// Select the best entry point and return.
		rax *filtered_entities = FilterTree_CollectModified(filters);
		select_entry_point(exps[0], filtered_entities, bound_vars);
		raxFree(filtered_entities);
		return;
	}

	// Compute all possible permutations of algebraic expressions.
	Arrangement *arrangements = permutations(exps, exps_count);
	uint arrangement_count = array_len(arrangements);
	if(arrangement_count == 1) goto cleanup;

	// Remove invalid arrangements.
	Arrangement *valid_arrangements = array_new(Arrangement, arrangement_count);
	for(int i = 0; i < arrangement_count; i++) {
		if(valid_arrangement(arrangements[i], exps_count)) {
			valid_arrangements = array_append(valid_arrangements, arrangements[i]);
		}
	}
	uint valid_arrangement_count = array_len(valid_arrangements);
	assert(valid_arrangement_count > 0);

	// Collect all filtered aliases.
	rax *filtered_entities = FilterTree_CollectModified(filters);

	/* Score each arrangement,
	 * keep track after arrangement with highest score. */
	int max_score = INT_MIN;
	Arrangement top_arrangement = valid_arrangements[0];

	for(uint i = 0; i < valid_arrangement_count; i++) {
		Arrangement arrangement = valid_arrangements[i];
		int score = _score_arrangement(arrangement, exps_count, filtered_entities, bound_vars);
		// printf("score: %d\n", score);
		// Arrangement_Print(arrangement, exps_count);
		if(max_score < score) {
			max_score = score;
			top_arrangement = arrangement;
		}
	}

	array_free(valid_arrangements);

	// Update input.
	for(uint i = 0; i < exps_count; i++) exps[i] = top_arrangement[i];

	// Transpose the winning expression if the destination node is a more efficient starting place.
	select_entry_point(exps[0], filtered_entities, bound_vars);

	// Depending on how the expressions have been ordered, we may have to transpose expressions
	// so that their source nodes have already been resolved by previous expressions.
	resolve_winning_sequence(exps, exps_count);

	raxFree(filtered_entities);
cleanup:
	for(uint i = 0; i < arrangement_count; i++) Arrangement_Free(arrangements[i]);
	array_free(arrangements);
}

