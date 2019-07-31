/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"
#include "../../arithmetic/arithmetic_expression.h"

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

int CondTraverseToString(const OpBase *ctx, char *buff, uint buff_len) {
	const CondTraverse *op = (const CondTraverse *)ctx;

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "%s | ", op->op.name);
	offset += QGNode_ToString(op->ae->src_node, buff + offset, buff_len - offset);
	if(op->ae->edge) {
		if(op->ae->operands[0].transpose) {
			offset += snprintf(buff + offset, buff_len - offset, "<-");
			offset += QGEdge_ToString(op->ae->edge, buff + offset, buff_len - offset);
			offset += snprintf(buff + offset, buff_len - offset, "-");
		} else {
			offset += snprintf(buff + offset, buff_len - offset, "-");
			offset += QGEdge_ToString(op->ae->edge, buff + offset, buff_len - offset);
			offset += snprintf(buff + offset, buff_len - offset, "->");
		}
	} else {
		offset += snprintf(buff + offset, buff_len - offset, "->");
	}
	offset += QGNode_ToString(op->ae->dest_node, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewCondTraverseOp(Graph *g, RecordMap *record_map, AlgebraicExpression *ae,
						  uint records_cap) {
	CondTraverse *traverse = calloc(1, sizeof(CondTraverse));
	traverse->graph = g;
	traverse->ae = ae;
	traverse->edgeRelationTypes = NULL;
	traverse->F = NULL;
	traverse->iter = NULL;
	traverse->edges = NULL;
	traverse->r = NULL;

	// Make sure that all entities are represented in Record
	traverse->srcNodeIdx = RecordMap_FindOrAddID(record_map, ae->src_node->id);
	traverse->destNodeIdx = RecordMap_FindOrAddID(record_map, ae->dest_node->id);
	traverse->edgeRecIdx = IDENTIFIER_NOT_FOUND;

	traverse->recordsLen = 0;
	traverse->transposed_edge = false;
	traverse->recordsCap = records_cap;
	traverse->records = rm_calloc(traverse->recordsCap, sizeof(Record));
	size_t required_dim = Graph_RequiredMatrixDim(g);
	GrB_Matrix_new(&traverse->M, GrB_BOOL, traverse->recordsCap, required_dim);
	GrB_Matrix_new(&traverse->F, GrB_BOOL, traverse->recordsCap, required_dim);

	// Set our Op operations
	OpBase_Init(&traverse->op);
	traverse->op.name = "Conditional Traverse";
	traverse->op.type = OPType_CONDITIONAL_TRAVERSE;
	traverse->op.consume = CondTraverseConsume;
	traverse->op.init = CondTraverseInit;
	traverse->op.reset = CondTraverseReset;
	traverse->op.toString = CondTraverseToString;
	traverse->op.free = CondTraverseFree;
	traverse->op.modifies = array_new(uint, 1);
	traverse->op.modifies = array_append(traverse->op.modifies, traverse->destNodeIdx);

	if(ae->edge) {
		traverse->edgeRecIdx = RecordMap_FindOrAddID(record_map, ae->edge->id);
		_setupTraversedRelations(traverse, ae->edge);
		traverse->op.modifies = array_append(traverse->op.modifies, traverse->edgeRecIdx);
		traverse->edges = array_new(Edge, 32);
	}

	return (OpBase *)traverse;
}

OpResult CondTraverseInit(OpBase *opBase) {
	CondTraverse *op = (CondTraverse *)opBase;
	size_t op_idx = 0;
	AlgebraicExpression *exp = op->ae;
	// If the input is set to be transposed on the first expression evaluation,
	// the source and destination nodes will be swapped in the record.
	op->transposed_edge = exp->edge && exp->operands[op_idx].transpose;

	return OP_OK;
}

/* CondTraverseConsume next operation
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
Record CondTraverseConsume(OpBase *opBase) {
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

OpResult CondTraverseReset(OpBase *ctx) {
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
void CondTraverseFree(OpBase *ctx) {
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
