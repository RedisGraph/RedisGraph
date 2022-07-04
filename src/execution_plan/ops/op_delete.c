/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/qsort.h"
#include "../../graph/graph_hub.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record DeleteConsume(OpBase *opBase);
static OpResult DeleteInit(OpBase *opBase);
static OpBase *DeleteClone(const ExecutionPlan *plan, const OpBase *opBase);
static OpResult DeleteReset(OpBase *opBase);
static void DeleteFree(OpBase *opBase);

void _DeleteEntities(OpDelete *op) {
	uint   edge_deleted           =  0;
	uint   implicit_edge_deleted  =  0;
	uint   node_count             =  array_len(op->deleted_nodes);
	uint   edge_count             =  array_len(op->deleted_edges);

	// nothing to delete, quickly return
	if((node_count + edge_count) == 0) return;

	//--------------------------------------------------------------------------
	// removing duplicates
	//--------------------------------------------------------------------------

	// remove node duplicates
	Node *nodes = op->deleted_nodes;
	Node *distinct_nodes = array_new(Node, 1);

#define is_entity_lt(a, b) (ENTITY_GET_ID((a)) < ENTITY_GET_ID((b)))
	QSORT(Node, nodes, node_count, is_entity_lt);

	for(uint i = 0; i < node_count; i++) {
		while(i < node_count - 1 && ENTITY_GET_ID(nodes + i) == ENTITY_GET_ID(nodes + i + 1)) i++;

		if(DataBlock_ItemIsDeleted((nodes + i)->attributes)) continue;
		array_append(distinct_nodes, *(nodes + i));
	}

	node_count = array_len(distinct_nodes);

	// remove edge duplicates
	Edge *edges = op->deleted_edges;
	Edge *distinct_edges = array_new(Edge, 1);

	QSORT(Edge, edges, edge_count, is_entity_lt);

	for(uint i = 0; i < edge_count; i++) {
		while(i < edge_count - 1 && ENTITY_GET_ID(edges + i) == ENTITY_GET_ID(edges + i + 1)) i++;

		array_append(distinct_edges, *(edges + i));
	}

	edge_count = array_len(distinct_edges);

	// lock everything
	QueryCtx_LockForCommit(); {
		// delete edges
		for(uint i = 0; i < edge_count; i++) {
			edge_deleted += DeleteEdge(op->gc, distinct_edges + i);
		}

		// delete nodes
		for(uint i = 0; i < node_count; i++) {
			implicit_edge_deleted += DeleteNode(op->gc, distinct_nodes + i);
		}

		// stats must be updated befor releasing the commit for replication
		if(op->stats != NULL) {
			op->stats->nodes_deleted          +=  node_count;
			op->stats->relationships_deleted  +=  edge_deleted;
			op->stats->relationships_deleted  +=  implicit_edge_deleted;
		}
	}
	// release lock
	QueryCtx_UnlockCommit(&op->op);

	// clean up
	array_free(distinct_nodes);
	array_free(distinct_edges);
}

OpBase *NewDeleteOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	OpDelete *op = rm_malloc(sizeof(OpDelete));

	op->gc = QueryCtx_GetGraphCtx();
	op->exps = exps;
	op->stats = NULL;
	op->exp_count = array_len(exps);
	op->deleted_nodes = array_new(Node, 32);
	op->deleted_edges = array_new(Edge, 32);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_DELETE, "Delete", DeleteInit, DeleteConsume,
				DeleteReset, NULL, DeleteClone, DeleteFree, true, plan);

	return (OpBase *)op;
}

static OpResult DeleteInit(OpBase *opBase) {
	OpDelete *op = (OpDelete *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	return OP_OK;
}

static Record DeleteConsume(OpBase *opBase) {
	OpDelete *op = (OpDelete *)opBase;
	OpBase *child = op->op.children[0];

	Record r = OpBase_Consume(child);
	if(!r) return NULL;

	/* Expression should be evaluated to either a node or an edge
	 * which will be marked for deletion, if an expression is evaluated
	 * to a different value type e.g. Numeric a run-time expection is thrown. */
	for(int i = 0; i < op->exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		SIValue value = AR_EXP_Evaluate(exp, r);
		SIType type = SI_TYPE(value);
		/* Enqueue entities for deletion. */
		if(type & T_NODE) {
			Node *n = (Node *)value.ptrval;
			array_append(op->deleted_nodes, *n);
			// If evaluating the expression allocated any memory, free it.
			SIValue_Free(value);
		} else if(type & T_EDGE) {
			Edge *e = (Edge *)value.ptrval;
			array_append(op->deleted_edges, *e);
			// If evaluating the expression allocated any memory, free it.
			SIValue_Free(value);
		} else if(!(type & T_NULL)) {
			/* Expression evaluated to a non-graph entity type
			 * clear pending deletions and raise an exception. */
			array_clear(op->deleted_nodes);
			array_clear(op->deleted_edges);
			// If evaluating the expression allocated any memory, free it.
			SIValue_Free(value);
			// free the Record this operation acted on
			OpBase_DeleteRecord(r);
			ErrorCtx_RaiseRuntimeException("Delete type mismatch, expecting either Node or Relationship.");
			break;
		}
	}

	return r;
}

static OpBase *DeleteClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_DELETE);
	OpDelete *op = (OpDelete *)opBase;
	AR_ExpNode **exps;
	array_clone_with_cb(exps, op->exps, AR_EXP_Clone);
	return NewDeleteOp(plan, exps);
}

static OpResult DeleteReset(OpBase *opBase) {
	OpDelete *op = (OpDelete *)opBase;

	_DeleteEntities(op);

	if(op->deleted_nodes) {
		array_clear(op->deleted_nodes);
	}

	if(op->deleted_edges) {
		array_clear(op->deleted_edges);
	}

	return OP_OK;
}

static void DeleteFree(OpBase *opBase) {
	OpDelete *op = (OpDelete *)opBase;

	_DeleteEntities(op);

	if(op->deleted_nodes) {
		array_free(op->deleted_nodes);
		op->deleted_nodes = NULL;
	}

	if(op->deleted_edges) {
		array_free(op->deleted_edges);
		op->deleted_edges = NULL;
	}

	if(op->exps) {
		for(int i = 0; i < op->exp_count; i++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}

