/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_cond_var_len_traverse.h"
#include "shared/print_functions.h"
#include "../../util/arr.h"
#include "../../ast/ast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "../../algorithms/all_neighbors.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static void CondVarLenTraverseFree(OpBase *opBase);

// Set the traversal direction to match the traversed edge and AlgebraicExpression form.
static inline void _setTraverseDirection(CondVarLenTraverse *op, const QGEdge *e) {
	if(e->bidirectional) {
		op->traverseDir = GRAPH_EDGE_DIR_BOTH;
	} else {
		if(AlgebraicExpression_Transposed(op->ae)) {
			// traverse in the opposite direction, (dest)->(src) incoming edges
			op->traverseDir = GRAPH_EDGE_DIR_INCOMING;
		} else {
			op->traverseDir = GRAPH_EDGE_DIR_OUTGOING;
		}
	}
}

static inline void CondVarLenTraverseToString(const OpBase *ctx, sds *buf) {
	// TODO: tmp, improve TraversalToString
	AlgebraicExpression_Optimize(&((CondVarLenTraverse *)ctx)->ae);
	return TraversalToString(ctx, buf, ((const CondVarLenTraverse *)ctx)->ae);
}

void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op) {
	// Expand into doesn't performs any modifications.
	array_clear(op->op.modifies);
	op->expandInto = true;
	op->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO;
	op->op.name = "Conditional Variable Length Traverse (Expand Into)";
}

inline void CondVarLenTraverseOp_SetFilter(CondVarLenTraverse *op,
										   FT_FilterNode *ft) {
	ASSERT(op != NULL);
	ASSERT(ft != NULL);
	ASSERT(op->ft == NULL);

	op->ft = ft;
}

OpBase *NewCondVarLenTraverseOp(const ExecutionPlan *plan, AlgebraicExpression *ae) {
	ASSERT(ae != NULL);

	CondVarLenTraverse *op = rm_malloc(sizeof(CondVarLenTraverse));
	op->ae                 =  ae;
	op->ft                 =  NULL;
	op->expandInto         =  false;
	op->collect_paths      =  true;
	op->edgeRelationTypes  =  NULL;

	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_VAR_LEN_TRAVERSE,
				"Conditional Variable Length Traverse", CondVarLenTraverseToString,
				CondVarLenTraverseFree, false, plan);

	// populate edge value in record only if it is referenced
	AST *ast = QueryCtx_GetAST();
	QGEdge *e = QueryGraph_GetEdgeByAlias(plan->query_graph, AlgebraicExpression_Edge(op->ae));
	_setTraverseDirection(op, e);

	OpBase_Modifies((OpBase *)op, AlgebraicExpression_Destination(ae));

	return (OpBase *)op;
}

static void CondVarLenTraverseFree(OpBase *ctx) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)ctx;

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->ft) {
		FilterTree_Free(op->ft);
		op->ft = NULL;
	}
}
