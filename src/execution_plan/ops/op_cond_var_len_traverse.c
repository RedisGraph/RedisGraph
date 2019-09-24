/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "../../util/arr.h"
#include "../../ast/ast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "./op_cond_var_len_traverse.h"

static void _setupTraversedRelations(CondVarLenTraverse *op, QGEdge *e) {
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

int CondVarLenTraverseToString(const OpBase *ctx, char *buff, uint buff_len) {
	const CondVarLenTraverse *op = (const CondVarLenTraverse *)ctx;

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

void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op) {
	// Expand into doesn't performs any modifications.
	array_clear(op->op.modifies);
	op->expandInto = true;
	op->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO;
	op->op.name = "Conditional Variable Length Traverse (Expand Into)";
}

OpBase *NewCondVarLenTraverseOp(Graph *g, RecordMap *record_map, AlgebraicExpression *ae) {
	assert(ae && ae->edge->minHops <= ae->edge->maxHops && g && ae->operand_count == 1);

	CondVarLenTraverse *condVarLenTraverse = malloc(sizeof(CondVarLenTraverse));
	condVarLenTraverse->g = g;
	condVarLenTraverse->ae = ae;
	condVarLenTraverse->expandInto = false;
	condVarLenTraverse->edgeRelationTypes = NULL;

	condVarLenTraverse->srcNodeIdx = RecordMap_FindOrAddID(record_map, ae->src_node->id);
	condVarLenTraverse->destNodeIdx = RecordMap_FindOrAddID(record_map, ae->dest_node->id);

	condVarLenTraverse->minHops = ae->edge->minHops;
	condVarLenTraverse->maxHops = ae->edge->maxHops;
	condVarLenTraverse->allPathsCtx = NULL;
	// The AlgebraicExpression populating a variable-length traversal only contains one operand.
	condVarLenTraverse->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING :
									  GRAPH_EDGE_DIR_OUTGOING;
	condVarLenTraverse->r = NULL;

	_setupTraversedRelations(condVarLenTraverse, ae->edge);

	// Set our Op operations
	OpBase_Init(&condVarLenTraverse->op);
	condVarLenTraverse->op.name = "Conditional Variable Length Traverse";
	condVarLenTraverse->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
	condVarLenTraverse->op.consume = CondVarLenTraverseConsume;
	condVarLenTraverse->op.reset = CondVarLenTraverseReset;
	condVarLenTraverse->op.toString = CondVarLenTraverseToString;
	condVarLenTraverse->op.free = CondVarLenTraverseFree;

	condVarLenTraverse->op.modifies = array_new(uint, 1);
	condVarLenTraverse->op.modifies = array_append(condVarLenTraverse->op.modifies,
												   condVarLenTraverse->destNodeIdx);

	return (OpBase *)condVarLenTraverse;
}

Record CondVarLenTraverseConsume(OpBase *opBase) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)opBase;
	OpBase *child = op->op.children[0];
	Path p = NULL;

	/* Incase we don't have any relations to traverse we can return quickly
	 * Consider: MATCH (S)-[:L*]->(M) RETURN M
	 * where label L does not exists. */
	if(op->edgeRelationCount == 0) {
		return NULL;
	}

compute_path:
	while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
		Record childRecord = OpBase_Consume(child);
		if(!childRecord) return NULL;

		if(op->r) Record_Free(op->r);
		op->r = childRecord;

		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);

		AllPathsCtx_Free(op->allPathsCtx);
		op->allPathsCtx = AllPathsCtx_New(srcNode,
										  op->g,
										  op->edgeRelationTypes,
										  op->edgeRelationCount,
										  op->traverseDir,
										  op->minHops,
										  op->maxHops);
	}

	// For the timebeing we only care for the last node in path
	Node n = Path_head(p);

	if(op->expandInto) {
		/* Dest node is already resolved
		 * need to make sure src is connected to dest
		 * i.e. n == dest. */
		Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
		if(ENTITY_GET_ID(&n) != ENTITY_GET_ID(destNode)) goto compute_path;
	} else {
		Record_AddNode(op->r, op->destNodeIdx, n);
	}

	return Record_Clone(op->r);
}

OpResult CondVarLenTraverseReset(OpBase *ctx) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)ctx;
	if(op->r) Record_Free(op->r);
	op->r = NULL;
	AllPathsCtx_Free(op->allPathsCtx);
	op->allPathsCtx = NULL;
	return OP_OK;
}

void CondVarLenTraverseFree(OpBase *ctx) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)ctx;

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}

	if(op->allPathsCtx) {
		AllPathsCtx_Free(op->allPathsCtx);
		op->allPathsCtx = NULL;
	}
}

