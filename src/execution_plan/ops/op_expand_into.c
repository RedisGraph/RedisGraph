/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "../../ast/ast.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../query_ctx.h"
#include "../../GraphBLASExt/GxB_Delete.h"

// String representation of operation.
static int ExpandIntoToString(const OpBase *ctx, char *buff, uint buff_len) {
	const OpExpandInto *op = (const OpExpandInto *)ctx;

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "%s | ", op->op.name);
	offset += QGNode_ToString(op->ae->src_node, buff + offset, buff_len - offset);
	if(op->ae->edge) {
		offset += snprintf(buff + offset, buff_len - offset, "-");
		offset += QGEdge_ToString(op->ae->edge, buff + offset, buff_len - offset);
		offset += snprintf(buff + offset, buff_len - offset, "->");
	} else {
		offset += snprintf(buff + offset, buff_len - offset, "->");
	}
	offset += QGNode_ToString(op->ae->dest_node, buff + offset, buff_len - offset);
	return offset;
}

/* Collects traversed edge relations.
 * e.g. [e:R0|R1]
 * op->edgeRelationTypes will hold both R0 and R1 IDs.
 * in the case where no relationship types are specified
 * op->edgeRelationTypes will contain GRAPH_NO_RELATION. */
static void _setupTraversedRelations(OpExpandInto *op, QGEdge *e) {
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

// Sets traversed edge within record.
static bool _setEdge(OpExpandInto *op) {
	// Consumed edges connecting current source and destination nodes.
	if(!array_len(op->edges)) return false;

	Edge *e = op->edges + (array_len(op->edges) - 1);
	Record_AddEdge(op->r, op->edgeIdx, *e);
	array_pop(op->edges);
	return true;
}

/* Evaluate algebraic expression:
 * appends filter matrix as the left most operand
 * perform multiplications.
 * removed filter matrix from original expression
 * clears filter matrix. */
static void _traverse(OpExpandInto *op) {
	// Append filter matrix to algebraic expression, as the left most operand.
	AlgebraicExpression_PrependTerm(op->ae, op->F, false, false, false);

	// Evaluate expression.
	AlgebraicExpression_Execute(op->ae, op->M);

	// Remove operand.
	AlgebraicExpression_RemoveTerm(op->ae, 0, NULL);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}

OpBase *NewExpandIntoOp(Graph *g, RecordMap *record_map, AlgebraicExpression *ae,
						uint records_cap) {
	OpExpandInto *expandInto = calloc(1, sizeof(OpExpandInto));
	expandInto->graph = g;
	expandInto->ae = ae;
	expandInto->edgeRelationTypes = NULL;
	expandInto->F = NULL;
	expandInto->edges = NULL;
	expandInto->r = NULL;

	// Make sure that all entities are represented in Record
	expandInto->srcNodeIdx = RecordMap_FindOrAddID(record_map, ae->src_node->id);
	expandInto->destNodeIdx = RecordMap_FindOrAddID(record_map, ae->dest_node->id);
	expandInto->edgeIdx = IDENTIFIER_NOT_FOUND;

	expandInto->recordCount = 0;
	expandInto->recordsCap = records_cap;
	expandInto->records = rm_calloc(expandInto->recordsCap, sizeof(Record));

	// Set our Op operations
	OpBase_Init(&expandInto->op);
	expandInto->op.name = "Expand Into";
	expandInto->op.type = OPType_EXPAND_INTO;
	expandInto->op.init = ExpandIntoInit;
	expandInto->op.free = ExpandIntoFree;
	expandInto->op.reset = ExpandIntoReset;
	expandInto->op.consume = ExpandIntoConsume;
	expandInto->op.toString = ExpandIntoToString;
	expandInto->op.modifies = NULL;

	if(ae->edge) {
		expandInto->edgeIdx = RecordMap_FindOrAddID(record_map, ae->edge->id);
		_setupTraversedRelations(expandInto, ae->edge);
		expandInto->op.modifies = array_new(uint, 1);
		expandInto->op.modifies = array_append(expandInto->op.modifies, expandInto->edgeIdx);
		expandInto->edges = array_new(Edge, 32);
	}

	return (OpBase *)expandInto;
}

OpResult ExpandIntoInit(OpBase *opBase) {
	OpExpandInto *op = (OpExpandInto *)opBase;

	size_t required_dim = Graph_RequiredMatrixDim(op->graph);
	GrB_Matrix_new(&op->M, GrB_BOOL, op->recordsCap, required_dim);
	GrB_Matrix_new(&op->F, GrB_BOOL, op->recordsCap, required_dim);
	return OP_OK;
}

/* Emits a record when possible,
 * Returns NULL when we've got no more records. */
static Record _handoff(OpExpandInto *op) {
	/* If we're required to update edge,
	 * try to get an edge, if successful we can return quickly,
	 * otherwise try to get a new pair of source and destination nodes. */
	if(op->ae->edge) {
		if(_setEdge(op)) return Record_Clone(op->r);
	}

	Node *srcNode;
	Node *destNode;
	NodeID srcId = INVALID_ENTITY_ID;
	NodeID destId = INVALID_ENTITY_ID;

	/* Find a record where both record's source and destination
	 * nodes are connected. */
	while(op->recordCount) {
		op->recordCount--;
		// Current record resides at row recordCount.
		int rowIdx = op->recordCount;
		op->r = op->records[op->recordCount];

		srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		destNode = Record_GetNode(op->r, op->destNodeIdx);
		srcId = ENTITY_GET_ID(srcNode);
		destId = ENTITY_GET_ID(destNode);
		bool x;
		GrB_Info res = GrB_Matrix_extractElement_BOOL(&x, op->M, rowIdx, destId);
		// Src is not connected to dest.
		if(res != GrB_SUCCESS) continue;

		// If we're here src is connected to dest.
		if(op->ae->edge) {
			for(int i = 0; i < op->edgeRelationCount; i++) {
				Graph_GetEdgesConnectingNodes(op->graph,
											  srcId,
											  destId,
											  op->edgeRelationTypes[i],
											  &op->edges);
			}
			_setEdge(op);
			return Record_Clone(op->r);
		}

		// Mark as NULL to avoid double free.
		op->records[op->recordCount] = NULL;
		return op->r;
	}

	// Didn't manage to emit record.
	return NULL;
}

/* ExpandIntoConsume next operation
 * returns OP_DEPLETED when no additional updates are available */
Record ExpandIntoConsume(OpBase *opBase) {
	Node *n;
	Record r;
	OpExpandInto *op = (OpExpandInto *)opBase;
	OpBase *child = op->op.children[0];

	// As long as we don't have a record to emit.
	while((r = _handoff(op)) == NULL) {
		/* If we're here, we didn't managed to emit a record,
		 * clean up and try to get new data points. */
		for(int i = 0; i < op->recordsCap; i++) {
			if(op->records[i]) {
				Record_Free(op->records[i]);
				op->records[i] = NULL;
			} else {
				break;
			}
		}

		// Ask child operations for data.
		for(op->recordCount = 0; op->recordCount < op->recordsCap; op->recordCount++) {
			Record childRecord = OpBase_Consume(child);
			// Did not managed to get new data, break.
			if(!childRecord) break;

			// Store received record.
			op->records[op->recordCount] = childRecord;
			/* Update filter matrix F, set row i at position srcId
			 * F[i, srcId] = true. */
			n = Record_GetNode(childRecord, op->srcNodeIdx);
			NodeID srcId = ENTITY_GET_ID(n);
			GrB_Matrix_setElement_BOOL(op->F, true, op->recordCount, srcId);
		}

		// Depleted.
		if(op->recordCount == 0) return NULL;
		_traverse(op);
	}

	return r;
}

OpResult ExpandIntoReset(OpBase *ctx) {
	OpExpandInto *op = (OpExpandInto *)ctx;
	for(int i = 0; i < op->recordCount; i++) {
		if(op->records[i]) Record_Free(op->records[i]);
		op->records = NULL;
	}
	op->recordCount = 0;

	if(op->F) GrB_Matrix_clear(op->F);
	if(op->edges) array_clear(op->edges);
	return OP_OK;
}

/* Frees ExpandInto */
void ExpandIntoFree(OpBase *ctx) {
	OpExpandInto *op = (OpExpandInto *)ctx;
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

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->records) {
		for(int i = 0; i < op->recordsCap; i++) {
			if(op->records[i]) Record_Free(op->records[i]);
			else break;
		}
		rm_free(op->records);
		op->records = NULL;
	}
}

