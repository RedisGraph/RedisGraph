/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../runtimes/interpreted/ops/op.h"
#include "../runtimes/interpreted/ops/op_sort.h"
#include "../runtimes/interpreted/ops/op_limit.h"
#include "../runtimes/interpreted/ops/op_expand_into.h"
#include "../runtimes/interpreted/ops/op_conditional_traverse.h"

/* applyLimit will traverse the given execution plan looking for Limit operations.
 * Once one is found, all relevant child operations (e.g. Sort) will be
 * notified about the current limit value.
 * This is beneficial as a number of different optimizations can be applied
 * once a limit is known. */

static void notify_limit(RT_OpBase *op, uint limit) {
	OPType t = op->op_desc->type;

	switch(t) {
		// reset limit on eager operation
		case OPType_MERGE:
		case OPType_CREATE:
		case OPType_UPDATE:
		case OPType_DELETE:
		case OPType_AGGREGATE:
			limit = UNLIMITED;
			break;
		case OPType_LIMIT:
			// update limit
			limit = ((RT_OpLimit *)op)->limit;
			break;
		case OPType_SORT:
			((RT_OpSort *)op)->limit = limit;
			break;
		case OPType_EXPAND_INTO:
			((RT_OpExpandInto *)op)->record_cap = limit;
			break;
		case OPType_CONDITIONAL_TRAVERSE:
			((RT_OpCondTraverse *)op)->record_cap = limit;
			break;
		default:
			break;
	}

	for(uint i = 0; i < op->childCount; i++) {
		notify_limit(op->children[i], limit);
	}
}

void applyLimit(RT_ExecutionPlan *plan) {
	notify_limit(plan->root, UNLIMITED);
}

