/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "project_functions.h"

#include "../../../deps/rax/rax.h"
#include "../../../util/rax_extensions.h"

/* Merge all order expressions that can operate on the input Record into the projections array, removing duplicates. */
void CombineProjectionArrays(AR_ExpNode ***project_exp_ptr, AR_ExpNode ***order_exp_ptr) {
	AR_ExpNode **project_exps = *project_exp_ptr;
	AR_ExpNode **order_exps = *order_exp_ptr;
	if(!order_exps) return;

	rax *projection_names = raxNew();
	uint order_count = array_len(order_exps);
	uint project_count = array_len(project_exps);

	// Add all WITH/RETURN projection names to rax.
	for(uint i = 0; i < project_count; i ++) {
		const char *name = project_exps[i]->resolved_name;
		raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
	}

	// Merge non-duplicate and independent order expressions into the projection array.
	for(int i = 0; i < order_count; i ++) {
		const char *name = order_exps[i]->resolved_name;
		/* If the ORDER BY alias is also in WITH/RETURN; this expression is redundant and can simply be removed. Ex:
		 * UNWIND [1,2] AS a RETURN a ORDER BY a
		 * Otherwise, enter this block. */
		if(raxFind(projection_names, (unsigned char *)name, strlen(name)) == raxNotFound) {
			rax *aliases_in_expression = raxNew();
			AR_EXP_CollectEntities(order_exps[i], aliases_in_expression);
			bool expression_is_dependent = raxIntersects(aliases_in_expression, projection_names);
			raxFree(aliases_in_expression);
			// If the ORDER expression relies on a value produced by a WITH/RETURN expression, do not migrate it.
			if(expression_is_dependent) continue;
			// Otherwise, it is independent and may be combined with the input expressions. Ex:
			// MATCH (a) RETURN a.name ORDER BY a.age
			project_exps = array_append(project_exps, order_exps[i]);
		}
		order_exps = array_del_fast(order_exps, i); // Remove the redundant expression.
		// Decrement the loop variables so that the next evaluation operates on the correct ORDER expression.
		i--;
		order_count--;
	}
	raxFree(projection_names);
	*project_exp_ptr = project_exps;
	// If the array of ORDER expressions is depleted, free and NULL-set it.
	if(array_len(order_exps) == 0) {
		array_free(order_exps);
		order_exps = NULL;
	}
	*order_exp_ptr = order_exps;
}

