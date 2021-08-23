/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "RG.h"
#include "shared/print_functions.h"
#include "../../query_ctx.h"

// default number of records to accumulate before traversing
#define BATCH_SIZE 16

/* Forward declarations. */
static void CondTraverseFree(OpBase *opBase);

static void CondTraverseToString(const OpBase *ctx, sds *buf) {
	TraversalToString(ctx, buf, ((const OpCondTraverse *)ctx)->ae);
}

OpBase *NewCondTraverseOp(const ExecutionPlan *plan, AlgebraicExpression *ae) {
	OpCondTraverse *op = rm_malloc(sizeof(OpCondTraverse));
	op->ae = ae;
	op->dest_label = NULL;
	op->dest_label_id = GRAPH_NO_LABEL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_TRAVERSE, "Conditional Traverse",
				CondTraverseToString, CondTraverseFree,
				false, plan);

	const char *dest = AlgebraicExpression_Destination(ae);
	OpBase_Modifies((OpBase *)op, dest);
	// Check the QueryGraph node and retrieve label data if possible.
	QGNode *dest_node = QueryGraph_GetNodeByAlias(plan->query_graph, dest);
	op->dest_label = dest_node->label;
	op->dest_label_id = dest_node->labelID;

	const char *edge = AlgebraicExpression_Edge(ae);
	if(edge) {
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		OpBase_Modifies((OpBase *)op, edge);
	}

	return (OpBase *)op;
}

/* Frees CondTraverse */
static void CondTraverseFree(OpBase *ctx) {
	OpCondTraverse *op = (OpCondTraverse *)ctx;
	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}
}
