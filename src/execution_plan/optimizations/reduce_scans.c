/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "./reduce_scans.h"
#include "../../util/arr.h"
#include "../ops/op_conditional_traverse.h"
#include <assert.h>

static void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
	// Return early if the scan has no child operations.
	if(scan->childCount == 0) return;

	// The scan operation should be operating on a single alias.
	assert(array_len(scan->modifies) == 1);
	const char *scanned_alias = scan->modifies[0];

	// Check if that alias is already bound by any of the scan's children.
	for(int i = 0; i < scan->childCount; i ++) {
		if(ExecutionPlan_LocateOpResolvingAlias(scan->children[i], scanned_alias)) {
			// The scanned alias is already bound, remove the redundant scan op.
			ExecutionPlan_RemoveOp(plan, scan);
			OpBase_Free(scan);
			break;
		}
	}
}

void reduceScans(ExecutionPlan *plan) {
	// Collect all SCAN operations within the execution plan.
	OpBase **scans = ExecutionPlan_LocateOps(plan->root,
											 (OPType_ALL_NODE_SCAN | OPType_NODE_BY_LABEL_SCAN |
											  OPType_INDEX_SCAN | OPType_NODE_BY_ID_SEEK));
	uint scan_count = array_len(scans);
	for(uint i = 0; i < scan_count; i++) {
		_reduceScans(plan, scans[i]);
	}

	array_free(scans);
}

