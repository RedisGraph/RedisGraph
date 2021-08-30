/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_cond_var_len_traverse.h"
#include "../../../ops/shared/print_functions.h"
#include "../../../../util/arr.h"
#include "../../../../ast/ast.h"
#include "../../../../arithmetic/arithmetic_expression.h"
#include "../../../../graph/graphcontext.h"
#include "../../../../algorithms/all_paths.h"
#include "../../../../algorithms/all_neighbors.h"
#include "../../../../query_ctx.h"

/* Forward declarations. */
static RT_OpResult CondVarLenTraverseInit(RT_OpBase *opBase);
static RT_OpResult CondVarLenTraverseReset(RT_OpBase *opBase);
static Record CondVarLenTraverseConsume(RT_OpBase *opBase);
static Record CondVarLenTraverseOptimizedConsume(RT_OpBase *opBase);
static void CondVarLenTraverseFree(RT_OpBase *opBase);

static void _setupTraversedRelations(RT_CondVarLenTraverse *op) {
	QGEdge *e = QueryGraph_GetEdgeByAlias(op->op.plan->plan_desc->query_graph, AlgebraicExpression_Edge(op->ae));
	ASSERT(e->minHops <= e->maxHops);
	op->minHops = e->minHops;
	op->maxHops = e->maxHops;

	uint reltype_count = QGEdge_RelationCount(e);
	if(reltype_count == 0) {
		op->edgeRelationCount = 1;
		op->edgeRelationTypes = array_new(int, 1);
		array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
	} else {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		op->edgeRelationCount = 0;
		op->edgeRelationTypes = array_new(int, reltype_count);

		for(int i = 0; i < reltype_count; i++) {
			int rel_id = e->reltypeIDs[i];
			if(rel_id != GRAPH_UNKNOWN_RELATION) {
				array_append(op->edgeRelationTypes, rel_id);
			} else {
				const char *rel_type = e->reltypes[i];
				Schema *s = GraphContext_GetSchema(gc, rel_type, SCHEMA_EDGE);
				if(s) array_append(op->edgeRelationTypes, s->id);
			}
		}

		op->edgeRelationCount = array_len(op->edgeRelationTypes);
	}
}

static inline void CondVarLenTraverseToString(const RT_OpBase *ctx, sds *buf) {
	// TODO: tmp, improve TraversalToString
	AlgebraicExpression_Optimize(&((RT_CondVarLenTraverse *)ctx)->ae);
	return TraversalToString(ctx, buf, ((const RT_CondVarLenTraverse *)ctx)->ae);
}

RT_OpBase *RT_NewCondVarLenTraverseOp(const RT_ExecutionPlan *plan, const CondVarLenTraverse *op_desc) {
	RT_CondVarLenTraverse *op = rm_malloc(sizeof(RT_CondVarLenTraverse));
	op->op_desc            =  op_desc;
	op->r                  =  NULL;
	op->M                  =  NULL;
	op->ae                 =  AlgebraicExpression_Clone(op_desc->ae);
	op->allPathsCtx        =  NULL;
	op->collect_paths      =  true;
	op->allNeighborsCtx    =  NULL;
	op->edgeRelationTypes  =  NULL;

	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op,
		CondVarLenTraverseToString, CondVarLenTraverseInit,
		CondVarLenTraverseConsume, CondVarLenTraverseReset, 
		CondVarLenTraverseFree, plan);

	bool aware = RT_OpBase_Aware((RT_OpBase *)op, AlgebraicExpression_Source(op->ae), &op->srcNodeIdx);
	UNUSED(aware);
	ASSERT(aware);
	aware = RT_OpBase_Aware((RT_OpBase *)op, AlgebraicExpression_Destination(op->ae), &op->destNodeIdx);
	ASSERT(aware);

	QGEdge *e = QueryGraph_GetEdgeByAlias(plan->plan_desc->query_graph, AlgebraicExpression_Edge(op->ae));
	if(!RT_OpBase_Aware((RT_OpBase *)op, e->alias, (uint *)&op->edgesIdx)) {
		op->edgesIdx = -1;
	};

	return (RT_OpBase *)op;
}

static RT_OpResult CondVarLenTraverseInit(RT_OpBase *opBase) {
	RT_CondVarLenTraverse *op = (RT_CondVarLenTraverse *)opBase;

	op->g =  QueryCtx_GetGraph();

	// check if variable length traversal doesn't require path construction
	// in which case we only care for reachable destination nodes
	// which is alot cheaper to compute
	//
	// consider:
	// MATCH (a)-[:L*2..4]->(b) RETURN b
	// we only care for destination nodes, the actual path is not of interest
	//
	// for this we require:
	// 1. no filters to be applied to pattern
	// 2. traversed edge isn't referenced
	// 3. traversal of a single relationship: R, RT
	// 4. traversal must be directed
	//
	// in which case we can use a faster consume function

	QGEdge *e = QueryGraph_GetEdgeByAlias(op->op.plan->plan_desc->query_graph,
			AlgebraicExpression_Edge(op->ae));
	uint reltype_count = QGEdge_RelationCount(e);

	bool  multi_edge  =  true;
	bool  transpose   =  op->op_desc->traverseDir != GRAPH_EDGE_DIR_OUTGOING;
	if(reltype_count == 1) {
		int rel_id = QGEdge_RelationID(e, 0);
		if(rel_id != GRAPH_NO_RELATION && rel_id != GRAPH_UNKNOWN_RELATION) {
			multi_edge = Graph_RelationshipContainsMultiEdge(op->g, rel_id,
					transpose);
		}
	}

	if(op->op_desc->ft          == NULL                && // no filter on path
	   op->edgesIdx             == -1                  && // edge isn't required
	   op->op_desc->expandInto  == false               && // destination unknown
	   reltype_count            == 1                   && // single relationship
	   multi_edge               == false               && // no multi edge entries
	   op->op_desc->traverseDir != GRAPH_EDGE_DIR_BOTH    // directed
	) {
		AlgebraicExpression_Optimize(&op->ae);
		ASSERT(op->ae->type == AL_OPERAND);
		op->collect_paths = false;
		RT_OpBase_UpdateConsume(opBase, CondVarLenTraverseOptimizedConsume);
	}

	return OP_OK;
}

static Record CondVarLenTraverseOptimizedConsume(RT_OpBase *opBase) {
	RT_CondVarLenTraverse  *op     = (RT_CondVarLenTraverse *)opBase;
	RT_OpBase              *child  =  op->op.children[0];
	Node                   dest    =  GE_NEW_NODE();
	EntityID               dest_id =  INVALID_ENTITY_ID;

	while ((dest_id = AllNeighborsCtx_NextNeighbor(op->allNeighborsCtx)) ==
			INVALID_ENTITY_ID) {
		Record childRecord = RT_OpBase_Consume(child);
		if(!childRecord) return NULL;

		if(op->r) RT_OpBase_DeleteRecord(op->r);
		op->r = childRecord;

		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		if(srcNode == NULL) {
			// the child Record may not contain the source node
			// in scenarios like a failed OPTIONAL MATCH
			// in this case, delete the Record and try again
			RT_OpBase_DeleteRecord(op->r);
			op->r = NULL;
			continue;
		}

		// create edge relation type array on first call to consume
		if(!op->edgeRelationTypes) {
			_setupTraversedRelations(op);
			// incase we don't have any relations to traverse
			// and minimal traversal is at least one hop
			// we can return quickly
			// consider: MATCH (S)-[:L*]->(M) RETURN M
			// where label L does not exists */
			if(op->edgeRelationCount == 0 && op->minHops > 0) return NULL;

			op->M = op->ae->operand.matrix;
		}

		if(op->allNeighborsCtx == NULL) {
			op->allNeighborsCtx = AllNeighborsCtx_New(srcNode->id, op->M,
					op->minHops, op->maxHops);
		} else {
			// in case ctx already allocated simply reset it
			AllNeighborsCtx_Reset(op->allNeighborsCtx, srcNode->id, op->M,
					op->minHops, op->maxHops);
		}
	}

	// could not produce destination node, return
	if(dest_id == INVALID_ENTITY_ID) return NULL;

	int res = Graph_GetNode(op->g, dest_id, &dest);
	UNUSED(res);
	ASSERT(res == true);

	//--------------------------------------------------------------------------
	// populate output record
	//--------------------------------------------------------------------------

	// add destination node to record
	Record r = RT_OpBase_CloneRecord(op->r);
	Record_AddNode(r, op->destNodeIdx, dest);

	return r;
}

static Record CondVarLenTraverseConsume(RT_OpBase *opBase) {
	RT_CondVarLenTraverse  *op     = (RT_CondVarLenTraverse *)opBase;
	Path                   *p      =  NULL;
	RT_OpBase              *child  =  op->op.children[0];

	while(!(p = AllPathsCtx_NextPath(op->allPathsCtx))) {
		Record childRecord = RT_OpBase_Consume(child);
		if(!childRecord) return NULL;

		if(op->r) RT_OpBase_DeleteRecord(op->r);
		op->r = childRecord;

		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		if(srcNode == NULL) {
			/* The child Record may not contain the source node in scenarios like
			 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
			RT_OpBase_DeleteRecord(op->r);
			op->r = NULL;
			continue;
		}

		// Create edge relation type array on first call to consume.
		if(!op->edgeRelationTypes) {
			_setupTraversedRelations(op);
			/* Incase we don't have any relations to traverse and minimal traversal is at least one hop
			 * we can return quickly.
			 * Consider: MATCH (S)-[:L*]->(M) RETURN M
			 * where label L does not exists. */
			if(op->edgeRelationCount == 0 && op->minHops > 0) return NULL;
		}

		Node *destNode = NULL;
		// The destination node is known in advance if we're performing an ExpandInto.
		if(op->op_desc->expandInto) destNode = Record_GetNode(op->r, op->destNodeIdx);

		AllPathsCtx_Free(op->allPathsCtx);
		op->allPathsCtx = AllPathsCtx_New(srcNode, destNode, op->g, op->edgeRelationTypes,
										  op->edgeRelationCount, op->op_desc->traverseDir, op->minHops,
										  op->maxHops, op->r, op->op_desc->ft, op->edgesIdx);

	}


	//--------------------------------------------------------------------------
	// populate output record
	//--------------------------------------------------------------------------

	Record r = RT_OpBase_CloneRecord(op->r);

	// add destination node to record
	if(!op->op_desc->expandInto) Record_AddNode(r, op->destNodeIdx, Path_Head(p));

	// add new path to record
	if(op->edgesIdx >= 0) Record_AddScalar(r, op->edgesIdx, SI_Path(p));

	return r;
}

static RT_OpResult CondVarLenTraverseReset(RT_OpBase *ctx) {
	RT_CondVarLenTraverse *op = (RT_CondVarLenTraverse *)ctx;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}

	if(op->collect_paths) {
		if(op->allPathsCtx) {
			AllPathsCtx_Free(op->allPathsCtx);
			op->allPathsCtx = NULL;
		}
	} else {
		if(op->allNeighborsCtx) {
			AllNeighborsCtx_Free(op->allNeighborsCtx);
			op->allNeighborsCtx = NULL;
		}
	}

	return OP_OK;
}

static void CondVarLenTraverseFree(RT_OpBase *ctx) {
	RT_CondVarLenTraverse *op = (RT_CondVarLenTraverse *)ctx;

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}

	if(op->collect_paths) {
		if(op->allPathsCtx) {
			AllPathsCtx_Free(op->allPathsCtx);
			op->allPathsCtx = NULL;
		}
	} else {
		if(op->allNeighborsCtx) {
			AllNeighborsCtx_Free(op->allNeighborsCtx);
			op->allNeighborsCtx = NULL;
		}
	}
}
