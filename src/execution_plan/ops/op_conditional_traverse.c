/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "shared/print_functions.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static OpResult CondTraverseInit(OpBase *opBase);
static Record CondTraverseConsume(OpBase *opBase);
static OpResult CondTraverseReset(OpBase *opBase);
static void CondTraverseFree(OpBase *opBase);

static void _setupTraversedRelations(CondTraverse *op, QGEdge *e) {
	uint reltype_count = array_len(e->reltypeIDs);
	if(reltype_count > 0) {
		array_clone(op->edgeRelationTypes, e->reltypeIDs);
		op->edgeRelationCount = reltype_count;
	} else {
		op->edgeRelationCount = 1;
		op->edgeRelationTypes = array_new(int, 1);
		op->edgeRelationTypes = array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
	}
}

/* Given an AlgebraicExpression with a populated edge, determine whether we're traversing a
 * transposed edge matrix. The edge matrix will be either the first or second operand, and is
 * the only operand which can be transposed (as the others are label diagonals). */
static inline bool _expressionContainsTranspose(AlgebraicExpression *exp) {
	for(uint i = 0; i < exp->operand_count; i ++) {
		if(exp->operands[i].transpose) {
			return true;
		}
	}
	return false;
}

// Updates query graph edge.
static int _CondTraverse_SetEdge(CondTraverse *op, Record r) {
	// Consumed edges connecting current source and destination nodes.
	if(!array_len(op->edges)) return 0;

	Edge *e = op->edges + (array_len(op->edges) - 1);
	Record_AddEdge(r, op->edgeRecIdx, *e);
	array_pop(op->edges);
	return 1;
}

/* Evaluate algebraic expression:
 * prepends filter matrix as the left most operand
 * perform multiplications
 * set iterator over result matrix
 * removed filter matrix from original expression
 * clears filter matrix. */
void _traverse(CondTraverse *op) {
	// Prepend matrix to algebraic expression, as the left most operand.
	AlgebraicExpression_PrependTerm(op->ae, op->F, false, false, false);
	// Evaluate expression.
	AlgebraicExpression_Execute(op->ae, op->M);

	// Remove operand.
	AlgebraicExpression_RemoveTerm(op->ae, 0, NULL);

	if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
	else GxB_MatrixTupleIter_reuse(op->iter, op->M);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}

static inline int CondTraverseToString(const OpBase *ctx, char *buf, uint buf_len) {
	return TraversalToString(ctx, buf, buf_len, ((const CondTraverse *)ctx)->ae);
}

OpBase *NewCondTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae,
						  uint records_cap) {
	CondTraverse *op = calloc(1, sizeof(CondTraverse));
	op->graph = g;
	op->ae = ae;
	op->r = NULL;
	op->F = NULL;
	op->iter = NULL;
	op->edges = NULL;
	op->recordsLen = 0;
	op->transposed_edge = false;
	op->edgeRelationTypes = NULL;
	op->recordsCap = records_cap;
	op->records = rm_calloc(op->recordsCap, sizeof(Record));

	size_t required_dim = Graph_RequiredMatrixDim(g);
	GrB_Matrix_new(&op->M, GrB_BOOL, op->recordsCap, required_dim);
	GrB_Matrix_new(&op->F, GrB_BOOL, op->recordsCap, required_dim);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_TRAVERSE, "Conditional Traverse", CondTraverseInit,
				CondTraverseConsume, CondTraverseReset, CondTraverseToString, CondTraverseFree, plan);

	assert(OpBase_Aware((OpBase *)op, ae->src, &op->srcNodeIdx));
	op->destNodeIdx = OpBase_Modifies((OpBase *)op, ae->dest);

	if(ae->edge) {
		op->edges = array_new(Edge, 32);
		QGEdge *qg_edge = QueryGraph_GetEdgeByAlias(plan->query_graph, ae->edge);
		_setupTraversedRelations(op, qg_edge);
		op->edgeRecIdx = OpBase_Modifies((OpBase *)op, ae->edge);
	}

	return (OpBase *)op;
}

static OpResult CondTraverseInit(OpBase *opBase) {
	CondTraverse *op = (CondTraverse *)opBase;
	AlgebraicExpression *exp = op->ae;

	// Nothing needs to be done if we're not populating an edge.
	if(exp->edge == NULL) return OP_OK;
	// If this operation traverses a transposed edge, the source and destination nodes
	// will be swapped in the Record.
	op->transposed_edge = _expressionContainsTranspose(exp);

	return OP_OK;
}

/* CondTraverseConsume next operation
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
static Record CondTraverseConsume(OpBase *opBase) {
	CondTraverse *op = (CondTraverse *)opBase;
	OpBase *child = op->op.children[0];

	/* If we're required to update edge,
	 * try to get an edge, if successful we can return quickly,
	 * otherwise try to get a new pair of source and destination nodes. */
	if(op->ae->edge) {
		if(_CondTraverse_SetEdge(op, op->r)) {
			return Record_Clone(op->r);
		}
	}

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
		for(int i = 0; i < op->recordsLen; i++) Record_Free(op->records[i]);

		// Ask child operations for data.
		for(op->recordsLen = 0; op->recordsLen < op->recordsCap; op->recordsLen++) {
			Record childRecord = OpBase_Consume(child);
			if(!childRecord) break;

			// Store received record.
			op->records[op->recordsLen] = childRecord;
			/* Update filter matrix F, set column i at position srcId
			 * F[srcId, i] = true. */
			Node *n = Record_GetNode(childRecord, op->srcNodeIdx);
			NodeID srcId = ENTITY_GET_ID(n);
			GrB_Matrix_setElement_BOOL(op->F, true, op->recordsLen, srcId);
		}

		// No data.
		if(op->recordsLen == 0) return NULL;

		_traverse(op);
	}

	/* Get node from current column. */
	op->r = op->records[src_id];
	Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
	Graph_GetNode(op->graph, dest_id, destNode);

	if(op->ae->edge) {
		// We're guarantee to have at least one edge.
		Node *srcNode;
		Node *destNode;

		if(op->transposed_edge) {
			srcNode = Record_GetNode(op->r, op->destNodeIdx);
			destNode = Record_GetNode(op->r, op->srcNodeIdx);
		} else {
			srcNode = Record_GetNode(op->r, op->srcNodeIdx);
			destNode = Record_GetNode(op->r, op->destNodeIdx);
		}

		for(int i = 0; i < op->edgeRelationCount; i++) {
			Graph_GetEdgesConnectingNodes(op->graph,
										  ENTITY_GET_ID(srcNode),
										  ENTITY_GET_ID(destNode),
										  op->edgeRelationTypes[i],
										  &op->edges);
		}

		_CondTraverse_SetEdge(op, op->r);
	}

	return Record_Clone(op->r);
}

static OpResult CondTraverseReset(OpBase *ctx) {
	CondTraverse *op = (CondTraverse *)ctx;
	if(op->r) Record_Free(op->r);
	if(op->edges) array_clear(op->edges);
	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}
	if(op->F) GrB_Matrix_clear(op->F);
	return OP_OK;
}

/* Frees CondTraverse */
static void CondTraverseFree(OpBase *ctx) {
	CondTraverse *op = (CondTraverse *)ctx;
	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}

	if(op->F) {
		GrB_Matrix_free(&op->F);
		op->F = NULL;
	}

	if(op->M) {
		GrB_Matrix_free(&op->M);
		op->M = NULL;
	}

	if(op->edges) {
		array_free(op->edges);
		op->edges = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->records) {
		for(int i = 0; i < op->recordsLen; i++) Record_Free(op->records[i]);
		rm_free(op->records);
		op->records = NULL;
	}
}

