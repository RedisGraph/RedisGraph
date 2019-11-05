/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "./reduce_scans.h"
#include "../../util/arr.h"
#include "../ops/op_conditional_traverse.h"
#include <assert.h>

// Locates all SCAN operations within execution plan.
void _collectScanOps(OpBase *root, OpBase ***scans) {
	if(root == NULL) return;

	if(root->type == OPType_ALL_NODE_SCAN ||
	   root->type == OPType_NODE_BY_LABEL_SCAN ||
	   root->type == OPType_INDEX_SCAN) {
		*scans = array_append(*scans, root);
	}

	for(int i = 0; i < root->childCount; i++) {
		_collectScanOps(root->children[i], scans);
	}
}

void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
	// Return early if the scan has no child operations.
	if(scan->childCount == 0) return;

	// Collect all variables bound before this scan.
	rax *bound_vars = raxNew();
	if(plan->root) ExecutionPlan_BoundVariables(scan->children[0], bound_vars);

	// See if one of the taps modifies the same entity as
	// current scan operation.
	assert(array_len(scan->modifies) == 1);
	const char *scanned_alias = scan->modifies[0];

	if(raxFind(bound_vars, (unsigned char *)scanned_alias, strlen(scanned_alias)) != raxNotFound) {
		// The scanned alias is already bound, remove the redundant scan op.
		ExecutionPlan_RemoveOp(plan, scan);
	}

	raxFree(bound_vars);
}

void reduceScans(ExecutionPlan *plan) {
	OpBase **scans = array_new(OpBase *, 0);
	_collectScanOps(plan->root, &scans);

	uint scan_count = array_len(scans);
	for(uint i = 0; i < scan_count; i++) {
		_reduceScans(plan, scans[i]);
	}

	array_free(scans);
}

