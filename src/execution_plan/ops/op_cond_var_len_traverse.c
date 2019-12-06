/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "op_cond_var_len_traverse.h"
#include "shared/print_functions.h"
#include "../../util/arr.h"
#include "../../ast/ast.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../graph/graphcontext.h"
#include "../../algorithms/all_paths.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static Record CondVarLenTraverseConsume(OpBase *opBase);
static OpResult CondVarLenTraverseReset(OpBase *opBase);
static void CondVarLenTraverseFree(OpBase *opBase);

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

static inline int CondVarLenTraverseToString(const OpBase *ctx, char *buf, uint buf_len) {
	return TraversalToString(ctx, buf, buf_len, ((const CondVarLenTraverse *)ctx)->ae);
}

void CondVarLenTraverseOp_ExpandInto(CondVarLenTraverse *op) {
	// Expand into doesn't performs any modifications.
	array_clear(op->op.modifies);
	op->expandInto = true;
	op->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO;
	op->op.name = "Conditional Variable Length Traverse (Expand Into)";
}

OpBase *NewCondVarLenTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae,
								QGEdge *edge) {
	assert(ae && edge->minHops <= edge->maxHops && g && ae->operand_count == 1);

	CondVarLenTraverse *op = malloc(sizeof(CondVarLenTraverse));
	op->g = g;
	op->ae = ae;
	op->r = NULL;
	op->expandInto = false;
	op->allPathsCtx = NULL;
	op->edgeRelationTypes = NULL;
	op->minHops = edge->minHops;
	op->maxHops = edge->maxHops;
	// The AlgebraicExpression populating a variable-length traversal only contains one operand.
	op->traverseDir = (ae->operands[0].transpose) ? GRAPH_EDGE_DIR_INCOMING : GRAPH_EDGE_DIR_OUTGOING;

	_setupTraversedRelations(op, edge);

	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_VAR_LEN_TRAVERSE,
				"Conditional Variable Length Traverse", NULL, CondVarLenTraverseConsume,
				CondVarLenTraverseReset, CondVarLenTraverseToString, CondVarLenTraverseFree, plan);
	assert(OpBase_Aware((OpBase *)op, ae->src, &op->srcNodeIdx));
	op->destNodeIdx = OpBase_Modifies((OpBase *)op, ae->dest);
	// Populate edge value in record only if it is referenced.
	AST *ast = QueryCtx_GetAST();
	if(AST_AliasIsReferenced(ast, edge->alias))
		op->edgesIdx = OpBase_Modifies((OpBase *)op, edge->alias);
	else op->edgesIdx = -1;

	return (OpBase *)op;
}

static Record CondVarLenTraverseConsume(OpBase *opBase) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)opBase;
	OpBase *child = op->op.children[0];
	Path *p = NULL;

	/* Incase we don't have any relations to traverse we can return quickly
	 * Consider: MATCH (S)-[:L*]->(M) RETURN M
	 * where label L does not exists. */
	if(op->edgeRelationCount == 0 || op->edgeRelationTypes[0] == GRAPH_UNKNOWN_RELATION) {
		return NULL;
	}

	while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
		Record childRecord = OpBase_Consume(child);
		if(!childRecord) return NULL;

		if(op->r) Record_Free(op->r);
		op->r = childRecord;

		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);

		AllPathsCtx_Free(op->allPathsCtx);
		if(op->expandInto) {
			Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
			op->allPathsCtx = AllPathsCtx_New(srcNode, destNode, op->g, op->edgeRelationTypes,
											  op->edgeRelationCount, op->traverseDir, op->minHops, op->maxHops);
		} else {
			op->allPathsCtx = AllPathsCtx_New(srcNode, NULL, op->g, op->edgeRelationTypes,
											  op->edgeRelationCount, op->traverseDir, op->minHops, op->maxHops);
		}

	}

	Node n = Path_Head(p);

	if(!op->expandInto) Record_AddNode(op->r, op->destNodeIdx, n);

	if(op->edgesIdx >= 0) Record_AddScalar(op->r, op->edgesIdx, SI_Path(p));

	return Record_Clone(op->r);
}

static OpResult CondVarLenTraverseReset(OpBase *ctx) {
	CondVarLenTraverse *op = (CondVarLenTraverse *)ctx;
	if(op->r) Record_Free(op->r);
	op->r = NULL;
	AllPathsCtx_Free(op->allPathsCtx);
	op->allPathsCtx = NULL;
	return OP_OK;
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

	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}

	if(op->allPathsCtx) {
		AllPathsCtx_Free(op->allPathsCtx);
		op->allPathsCtx = NULL;
	}
}

