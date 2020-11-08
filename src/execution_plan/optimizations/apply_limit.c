/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../ops/op.h"
#include "../ops/op_sort.h"
#include "../ops/op_limit.h"
#include "../ops/op_expand_into.h"
#include "../ops/op_conditional_traverse.h"

static void notify_limit(OpBase *op, uint limit) {
	OPType t = op->type;

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
		limit = ((OpLimit *)op)->limit;
		break;
	case OPType_SORT:
		((OpSort *)op)->limit = limit;
		break;
	case OPType_EXPAND_INTO:
		((OpExpandInto *)op)->record_cap = limit;
		break;
	case OPType_CONDITIONAL_TRAVERSE:
		((OpCondTraverse *)op)->record_cap = limit;
		break;
	default:
		break;
	}

	for(uint i = 0; i < op->childCount; i++) {
		notify_limit(op->children[i], limit);
	}
}

void applyLimit(ExecutionPlan *plan) {
	notify_limit(plan->root, UNLIMITED);
}

