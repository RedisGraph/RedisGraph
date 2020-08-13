/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "./reduce_scans.h"
#include "../../util/arr.h"
#include "../ops/op_conditional_traverse.h"
#include "../ops/op_node_by_label_scan.h"
#include "../ops/op_filter.h"
#include <assert.h>
#include "../../query_ctx.h"

static OpBase *_LabelScanToConditionalTraverse(NodeByLabelScan *label_scan) {
	AST *ast = QueryCtx_GetAST();
	Graph *g = QueryCtx_GetGraph();
	const QGNode *n = label_scan->n;
	AlgebraicExpression *ae = AlgebraicExpression_NewOperand(GrB_NULL, true, n->alias, n->alias, NULL,
															 n->label);
	return NewCondTraverseOp(label_scan->op.plan, g, ae);
}

static void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
	// Return early if the scan has no child operations.
	if(scan->childCount == 0) return;

	// The scan operation should be operating on a single alias.
	assert(array_len(scan->modifies) == 1);
	const char *scanned_alias = scan->modifies[0];

	// Collect variables bound before this operation.
	rax *bound_vars = raxNew();
	for(int i = 0; i < scan->childCount; i ++) {
		ExecutionPlan_BoundVariables(scan->children[i], bound_vars);
	}

	if(raxFind(bound_vars, (unsigned char *)scanned_alias, strlen(scanned_alias)) != raxNotFound) {
		// If the alias the scan operates on is already bound, the scan operation is redundant.
		if(scan->type == OPType_NODE_BY_LABEL_SCAN) {
			// If we are performing a label scan, introduce a conditional traversal to filter by label.
			OpBase *traverse = _LabelScanToConditionalTraverse((NodeByLabelScan *)scan);
			ExecutionPlan_ReplaceOp(plan, scan, traverse);
		} else {
			// Remove the redundant scan op.
			ExecutionPlan_RemoveOp(plan, scan);
		}
		OpBase_Free(scan);
	}
	raxFree(bound_vars);
}

void reduceScans(ExecutionPlan *plan) {
	// Collect all SCAN operations within the execution plan.
	OpBase **scans = ExecutionPlan_CollectOpsMatchingType(plan->root, SCAN_OPS, SCAN_OP_COUNT);
	uint scan_count = array_len(scans);
	for(uint i = 0; i < scan_count; i++) {
		_reduceScans(plan, scans[i]);
	}

	array_free(scans);
}

