/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../runtimes/interpreted/ops/op.h"
#include "../runtimes/interpreted/ops/op_sort.h"
#include "../runtimes/interpreted/ops/op_skip.h"

/* applySkip will traverse the given execution plan looking for Skip operations.
 * Once one is found, all relevant child operations (e.g. Sort) will be
 * notified about the current skip value.
 * This is beneficial as a number of different optimizations can be applied
 * once a skip is known */

static void notify_skip(RT_OpBase *op, uint skip) {
	OPType t = op->op_desc->type;

	switch(t) {
		case OPType_SKIP:
			// update skip
			skip = ((RT_OpSkip *)op)->skip;
			break;
		case OPType_SORT:
			((RT_OpSort *)op)->skip = skip;
			break;
		default:
			break;
	}

	for(uint i = 0; i < op->childCount; i++) {
		notify_skip(op->children[i], skip);
	}
}

void applySkip(RT_ExecutionPlan *plan) {
	notify_skip(plan->root, 0);
}

