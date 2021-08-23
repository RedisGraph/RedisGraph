/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "RG.h"
#include "shared/print_functions.h"
#include "../../ast/ast.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static void NodeByLabelScanFree(OpBase *opBase);

static inline void NodeByLabelScanToString(const OpBase *ctx, sds *buf) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	ScanToString(ctx, buf, op->n.alias, op->n.label);
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, NodeScanCtx n) {
	NodeByLabelScan *op = rm_malloc(sizeof(NodeByLabelScan));
	op->n = n;
	// Defaults to [0...UINT64_MAX].
	op->id_range = UnsignedRange_New();

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan",
				NodeByLabelScanToString, NodeByLabelScanFree, false, plan);

	OpBase_Modifies((OpBase *)op, n.alias);

	return (OpBase *)op;
}

void NodeByLabelScanOp_SetIDRange(NodeByLabelScan *op, UnsignedRange *id_range) {
	UnsignedRange_Free(op->id_range);
	op->id_range = UnsignedRange_Clone(id_range);

	op->op.type = OPType_NODE_BY_LABEL_AND_ID_SCAN;
	op->op.name = "Node By Label and ID Scan";
}

static void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;
	if(nodeByLabelScan->id_range) {
		UnsignedRange_Free(nodeByLabelScan->id_range);
		nodeByLabelScan->id_range = NULL;
	}
}
