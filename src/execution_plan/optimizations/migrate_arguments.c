/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "../ops/op_filter.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../execution_plan_build/execution_plan_modify.h"

/* the migrate arguments optimizer searches the execution plans for
 * argument operations that are the children of Cartesian Products
 * and relocates them to the tail of one of the other streams */

static void _migrateArguments(ExecutionPlan *plan, OpBase *argument) {
	// Return early if the Argument is not a child of a Cartesian Product
	OpBase *parent = argument->parent;
	if(parent->type != OPType_CARTESIAN_PRODUCT) return;

	// Collect variables bound in this Argument op.
	rax *bound_vars = raxNew();
	ExecutionPlan_BoundVariables(argument, bound_vars);

	// retrieve all scan ops in the Cartesian Product subtree
	OpBase **scans = ExecutionPlan_CollectOpsMatchingType(parent, SCAN_OPS, SCAN_OP_COUNT);
	uint scan_count = array_len(scans);
	for(uint i = 0; i < scan_count; i++) {
		OpBase *scan = scans[i];
		ASSERT(array_len(scan->modifies) == 1);
		const char *scanned_alias = scan->modifies[0];
		// if the scanned alias is already bound by the argument,
		// we should move the argument to be a child of the scan
		// the scan op will later be removed by reduceScans
		if(raxFind(bound_vars, (unsigned char *)scanned_alias,
				   strlen(scanned_alias)) != raxNotFound) {
			// migrate the original Argument op to be a child of the scan
			ExecutionPlan_RemoveOp(plan, argument);
			ExecutionPlan_AddOp(scan, argument);
			break;
		}
	}
	raxFree(bound_vars);
}

void migrateArguments(ExecutionPlan *plan) {
	// Collect all argument operations within the execution plan.
	OpBase **arguments = ExecutionPlan_CollectOps(plan->root, OPType_ARGUMENT);
	uint argument_count = array_len(arguments);
	for(uint i = 0; i < argument_count; i++) {
		_migrateArguments(plan, arguments[i]);
	}

	array_free(arguments);
}

