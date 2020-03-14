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
	return NewCondTraverseOp(label_scan->op.plan, g, ae, AR_EXP_Clone(ast->limit));
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
			/* Here is the case where a node is already populated in the record when it arrived to a label scan operation.
			 * The label scan operation here is redundent since it will re-scan the entire graph, so we will replace it with
			 * a conditional traverse operation which will filter only the nodes with the requested labels.
			 * The conditional traverse is done over the label matrix, and is more efficient then a filter operation. */
			if(scan->type == OPType_NODE_BY_LABEL_SCAN) {
				OpBase *traverse = _LabelScanToConditionalTraverse((NodeByLabelScan *)scan);
				ExecutionPlan_ReplaceOp(plan, scan, traverse);
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
	OpBase **scans = ExecutionPlan_CollectOpsMatchingType(plan->root, SCAN_OPS, SCAN_OP_COUNT);
	uint scan_count = array_len(scans);
	for(uint i = 0; i < scan_count; i++) {
		_reduceScans(plan, scans[i]);
	}

	array_free(scans);
}

