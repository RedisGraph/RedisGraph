/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "./reduce_scans.h"
#include "../../util/arr.h"
#include "../ops/op_conditional_traverse.h"
#include "../ops/op_node_by_label_scan.h"
#include "../ops/op_filter.h"
#include <assert.h>

static FT_FilterNode *buildLabelFilter(NodeByLabelScan *label_scan) {
	char *label = (char *)label_scan->n->label;
	AR_ExpNode *const_label_node = AR_EXP_NewConstOperandNode(SI_ConstStringVal(label));
	AR_ExpNode *variadic_node = AR_EXP_NewVariableOperandNode(label_scan->n->alias, NULL);
	AR_ExpNode *labels_func_node = AR_EXP_NewOpNode("labels", 1);
	labels_func_node->op.children[0] = variadic_node;
	return FilterTree_CreatePredicateFilter(OP_EQUAL, labels_func_node, const_label_node);
}

static void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
	// Return early if the scan has no child operations.
	if(scan->childCount == 0) return;

	// The scan operation should be operating on a single alias.
	assert(array_len(scan->modifies) == 1);
	const char *scanned_alias = scan->modifies[0];

	// Check if that alias is already bound by any of the scan's children.
	for(int i = 0; i < scan->childCount; i ++) {
		if(ExecutionPlan_LocateOpResolvingAlias(scan->children[i], scanned_alias)) {
			if(scan->type == OPType_NODE_BY_LABEL_SCAN) {
				OpBase *filter = NewFilterOp(plan, buildLabelFilter((NodeByLabelScan *)scan));
				ExecutionPlan_ReplaceOp(plan, scan, filter);
				OpBase_Free(scan);
				break;
			}
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

