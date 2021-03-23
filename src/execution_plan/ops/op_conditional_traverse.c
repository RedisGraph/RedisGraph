/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "RG.h"
#include "shared/print_functions.h"
#include "../../query_ctx.h"
#include "../../arithmetic/algebraic_expression/utils.h"

// default number of records to accumulate before traversing
#define BATCH_SIZE 16

/* Forward declarations. */
static OpResult CondTraverseInit(OpBase *opBase);
static Record CondTraverseConsume(OpBase *opBase);
static OpResult CondTraverseReset(OpBase *opBase);
static OpBase *CondTraverseClone(const ExecutionPlan *plan, const OpBase *opBase);
static void CondTraverseFree(OpBase *opBase);

static int CondTraverseToString(const OpBase *ctx, char *buf, uint buf_len) {
	return TraversalToString(ctx, buf, buf_len, ((const OpCondTraverse *)ctx)->ae);
}

static void _populate_filter_matrix(OpCondTraverse *op) {
	for(uint i = 0; i < op->record_count; i++) {
		Record r = op->records[i];
		/* Update filter matrix F, set row i at position srcId
		 * F[i, srcId] = true. */
		Node *n = Record_GetNode(r, op->srcNodeIdx);
		NodeID srcId = ENTITY_GET_ID(n);
		GrB_Matrix_setElement_BOOL(op->F, true, i, srcId);
	}
}

/* This function performs multiple-source algorithm that
 * solves context-free path reachability problem.
 * The algorithm description can be found here:
 * 		https://github.com/YaccConstructor/articles/blob/master/2021/EDBT/Multiple-source%20CFPQ/full/main.pdf
 */
static void _transitive_closure(PathPattern **deps, PathPatternCtx *pathPatternCtx, OpCondTraverse *op) {
	bool changed = true;

	size_t deps_size = array_len(deps);
	GrB_Index *nvals_ms = array_new(GrB_Index, deps_size);
	GrB_Index *nvals_srcs = array_new(GrB_Index, deps_size);

	GrB_Matrix *tmps = array_new(GrB_Matrix, deps_size);
	for (int i = 0; i < deps_size; ++i) {
		GrB_Index nrows, ncols;
		GrB_Matrix_nrows(&nrows, deps[i]->m);
		GrB_Matrix_ncols(&ncols, deps[i]->m);
		GrB_Matrix_new(&tmps[i], GrB_BOOL, nrows, ncols);
	}

	while (changed) {
		changed = false;
		for (int i = 0; i < deps_size; ++i) {
			GrB_Matrix_nvals(&nvals_ms[i], deps[i]->m);
			GrB_Matrix_nvals(&nvals_srcs[i], deps[i]->src);
		}

		for (int i = 0; i < array_len(deps); ++i) {
			PathPattern *pattern = deps[i];
			AlgebraicExpression_Eval(pattern->ae, tmps[i], pathPatternCtx);
			GrB_Matrix_eWiseAdd_BinaryOp(pattern->m, NULL, NULL, GrB_LOR, pattern->m, tmps[i], NULL);
		}

		for (int i = 0; i < deps_size; ++i) {
			GrB_Index nvals_m_new, nvals_src_new;
			GrB_Matrix_nvals(&nvals_m_new, deps[i]->m);
			GrB_Matrix_nvals(&nvals_src_new, deps[i]->src);

			if (nvals_ms[i] != nvals_m_new || nvals_srcs[i] != nvals_src_new) {
				changed = true;
			}
		}
	}

	for (int j = 0; j < deps_size; ++j) {
		GrB_Matrix_free(&tmps[j]);
	}
	array_free(tmps);
	array_free(nvals_srcs);
	array_free(nvals_ms);
}

/* Performs traverse along path pattern.
 * It behaves like _traverse function, but does some extra work related
 * to processing named path patterns.
 * */
void _traverse_path_pattern(OpCondTraverse *op) {
	// If op->F is null, this is the first time we are traversing.
	if(op->F == GrB_NULL) {
		// Create both filter and result matrices.
		size_t required_dim = Graph_RequiredMatrixDim(op->graph);
		GrB_Matrix_new(&op->M, GrB_BOOL, op->record_cap, required_dim);
		GrB_Matrix_new(&op->F, GrB_BOOL, op->record_cap, required_dim);

		// Prepend the filter matrix to algebraic expression as the leftmost operand.
		AlgebraicExpression_MultiplyToTheLeft(&op->ae, op->F);

		// Optimize the expression tree.
		AlgebraicExpression_Optimize(&op->ae);
		AlgebraicExpression_ReplaceTransposedReferences(op->ae);
		op->deps = PathPatternCtx_GetDependencies(op->path_pattern_ctx, op->ae);

		// Populate algebraic operand references with named path pattern matrices
		AlgebraicExpression_PopulateReferences(op->ae, op->path_pattern_ctx);
		for (int i = 0; i < array_len(op->deps); ++i) {
			AlgebraicExpression_PopulateReferences(op->deps[i]->ae, op->path_pattern_ctx);
		}
	}

	// Populate filter matrix.
	_populate_filter_matrix(op);

	// Clear named path patterns matrices
	PathPatternCtx_ClearMatrices(op->path_pattern_ctx);

	// Evaluate expression for construct sources of named path patterns
	AlgebraicExpression_Eval(op->ae, op->M, op->path_pattern_ctx);

	// Perform transitive closure of named path patterns.
	_transitive_closure(op->deps, op->path_pattern_ctx, op);

	// Evaluate expression. After transitive closure all named path
	// pattern matrices is completed and corresponds to the correct relation.
	AlgebraicExpression_Eval(op->ae, op->M, op->path_pattern_ctx);

	if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
	else GxB_MatrixTupleIter_reuse(op->iter, op->M);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}


/* Evaluate algebraic expression:
 * prepends filter matrix as the left most operand
 * perform multiplications
 * set iterator over result matrix
 * removed filter matrix from original expression
 * clears filter matrix. */
void _traverse(OpCondTraverse *op) {
	// If op->F is null, this is the first time we are traversing.
	if(op->F == GrB_NULL) {
		/* Create both filter and result matrices.
		 * make sure M's format is SPARSE, required by the matrix iterator */
		size_t required_dim = Graph_RequiredMatrixDim(op->graph);
		GrB_Matrix_new(&op->M, GrB_BOOL, op->record_cap, required_dim);
		GrB_Matrix_new(&op->F, GrB_BOOL, op->record_cap, required_dim);
		GxB_set(op->M, GxB_SPARSITY_CONTROL, GxB_SPARSE);

		// Prepend the filter matrix to algebraic expression as the leftmost operand.
		AlgebraicExpression_MultiplyToTheLeft(&op->ae, op->F);

		// Optimize the expression tree.
		AlgebraicExpression_Optimize(&op->ae);
	}

	// Populate filter matrix.
	_populate_filter_matrix(op);

	// Evaluate expression.
	AlgebraicExpression_Eval(op->ae, op->M, NULL);

	if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
	else GxB_MatrixTupleIter_reuse(op->iter, op->M);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}

OpBase *NewCondTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae, bool is_path_pattern) {
	OpCondTraverse *op = rm_malloc(sizeof(OpCondTraverse));
	op->graph = g;
	op->ae = ae;
	op->r = NULL;
	op->iter = NULL;
	op->F = GrB_NULL;
	op->M = GrB_NULL;
	op->records = NULL;
	op->record_count = 0;
	op->edge_ctx = NULL;
	op->dest_label = NULL;
	op->record_cap = BATCH_SIZE;
	op->dest_label_id = GRAPH_NO_LABEL;

	op->is_path_pattern = is_path_pattern;

	op->path_pattern_ctx = plan->path_pattern_ctx;
	// Dependencies to named path pattern initialized
	// in first time of consume operation.
	op->deps = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_TRAVERSE, "Conditional Traverse", CondTraverseInit,
				CondTraverseConsume, CondTraverseReset, CondTraverseToString, CondTraverseClone, CondTraverseFree,
				false, plan);

	bool aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Source(ae), &op->srcNodeIdx);
	UNUSED(aware);
	ASSERT(aware == true);

	const char *dest = AlgebraicExpression_Destination(ae);
	op->destNodeIdx = OpBase_Modifies((OpBase *)op, dest);
	// Check the QueryGraph node and retrieve label data if possible.
	QGNode *dest_node = QueryGraph_GetNodeByAlias(plan->query_graph, dest);
	op->dest_label = dest_node->label;
	op->dest_label_id = dest_node->labelID;

	const char *edge = AlgebraicExpression_Edge(ae);
	if(edge && !is_path_pattern) {
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		uint edge_idx = OpBase_Modifies((OpBase *)op, edge);
		QGEdge *e = QueryGraph_GetEdgeByAlias(plan->query_graph, edge);
		op->edge_ctx = Traverse_NewEdgeCtx(ae, e, edge_idx);
	}

	return (OpBase *)op;
}

static OpResult CondTraverseInit(OpBase *opBase) {
	OpCondTraverse *op = (OpCondTraverse *)opBase;
	// Create 'records' with this Init function as 'record_cap'
	// might be set during optimization time (applyLimit)
	// If cap greater than BATCH_SIZE is specified,
	// use BATCH_SIZE as the value.
	if(op->record_cap > BATCH_SIZE) op->record_cap = BATCH_SIZE;
	op->records = rm_calloc(op->record_cap, sizeof(Record));
	return OP_OK;
}

/* Each call to CondTraverseConsume emits a Record containing the
 * traversal's endpoints and, if required, an edge.
 * Returns NULL once all traversals have been performed. */
static Record CondTraverseConsume(OpBase *opBase) {
	OpCondTraverse *op = (OpCondTraverse *)opBase;
	OpBase *child = op->op.children[0];

	/* If we're required to update an edge and have one queued, we can return early.
	 * Otherwise, try to get a new pair of source and destination nodes. */
	if(op->edge_ctx && Traverse_SetEdge(op->edge_ctx, op->r)) return OpBase_CloneRecord(op->r);

	bool depleted = true;
	NodeID src_id = INVALID_ENTITY_ID;
	NodeID dest_id = INVALID_ENTITY_ID;

	while(true) {
		if(op->iter) GxB_MatrixTupleIter_next(op->iter, &src_id, &dest_id, &depleted);

		// Managed to get a tuple, break.
		if(!depleted) break;

		/* Run out of tuples, try to get new data.
		 * Free old records. */
		op->r = NULL;
		for(uint i = 0; i < op->record_count; i++) OpBase_DeleteRecord(op->records[i]);

		// Ask child operations for data.
		for(op->record_count = 0; op->record_count < op->record_cap; op->record_count++) {
			Record childRecord = OpBase_Consume(child);
			// If the Record is NULL, the child has been depleted.
			if(!childRecord) break;
			if(!Record_GetNode(childRecord, op->srcNodeIdx)) {
				/* The child Record may not contain the source node in scenarios like
				 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
				OpBase_DeleteRecord(childRecord);
				op->record_count--;
				continue;
			}

			// Store received record.
			Record_PersistScalars(childRecord);
			op->records[op->record_count] = childRecord;
		}

		// No data.
		if(op->record_count == 0) return NULL;

		if (op->is_path_pattern) {
			_traverse_path_pattern(op);
		} else {
			_traverse(op);
		}
	}

	/* Get node from current column. */
	op->r = op->records[src_id];
	/* Populate the destination node and add it to the Record.
	 * Note that if the node's label is unknown, this will correctly
	 * create an unlabeled node. */
	Node destNode = GE_NEW_LABELED_NODE(op->dest_label, op->dest_label_id);
	Graph_GetNode(op->graph, dest_id, &destNode);
	Record_AddNode(op->r, op->destNodeIdx, destNode);

	if(op->edge_ctx) {
		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		// Collect all appropriate edges connecting the current pair of endpoints.
		Traverse_CollectEdges(op->edge_ctx, ENTITY_GET_ID(srcNode), ENTITY_GET_ID(&destNode));
		// We're guaranteed to have at least one edge.
		Traverse_SetEdge(op->edge_ctx, op->r);
	}

	return OpBase_CloneRecord(op->r);
}

static OpResult CondTraverseReset(OpBase *ctx) {
	OpCondTraverse *op = (OpCondTraverse *)ctx;

	// Do not explicitly free op->r, as the same pointer is also held
	// in the op->records array and as such will be freed there.
	op->r = NULL;
	for(uint i = 0; i < op->record_count; i++) OpBase_DeleteRecord(op->records[i]);
	op->record_count = 0;

	if(op->edge_ctx) Traverse_ResetEdgeCtx(op->edge_ctx);

	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}
	if(op->F != GrB_NULL) GrB_Matrix_clear(op->F);
	return OP_OK;
}

static inline OpBase *CondTraverseClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_CONDITIONAL_TRAVERSE);
	OpCondTraverse *op = (OpCondTraverse *)opBase;
	return NewCondTraverseOp(plan, QueryCtx_GetGraph(), AlgebraicExpression_Clone(op->ae), op->is_path_pattern);
}

/* Frees CondTraverse */
static void CondTraverseFree(OpBase *ctx) {
	OpCondTraverse *op = (OpCondTraverse *)ctx;
	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}

	if(op->F != GrB_NULL) {
		GrB_Matrix_free(&op->F);
		op->F = GrB_NULL;
	}

	if(op->M != GrB_NULL) {
		GrB_Matrix_free(&op->M);
		op->M = GrB_NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->edge_ctx) {
		Traverse_FreeEdgeCtx(op->edge_ctx);
		op->edge_ctx = NULL;
	}

	if(op->records) {
		for(uint i = 0; i < op->record_count; i++) OpBase_DeleteRecord(op->records[i]);
		rm_free(op->records);
		op->records = NULL;
	}

	if (op->deps) {
		array_free(op->deps);
	}
}

