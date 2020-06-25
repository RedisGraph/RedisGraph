/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Forward declarations. */
static Record DeleteConsume(OpBase *opBase);
static OpResult DeleteInit(OpBase *opBase);
static OpBase *DeleteClone(const ExecutionPlan *plan, const OpBase *opBase);
static void DeleteFree(OpBase *opBase);

void _DeleteEntities(OpDelete *op) {
	Graph *g = op->gc->g;
	uint node_deleted = 0;
	uint relationships_deleted = 0;
	uint node_count = array_len(op->deleted_nodes);
	uint edge_count = array_len(op->deleted_edges);

	/* Nothing to delete, quickly return. */
	if((node_count + edge_count) == 0) goto cleanup;

	/* Lock everything. */
	QueryCtx_LockForCommit();

	if(GraphContext_HasIndices(op->gc)) {
		for(int i = 0; i < node_count; i++) {
			Node *n = op->deleted_nodes + i;
			GraphContext_DeleteNodeFromIndices(op->gc, n);
		}
	}

	Graph_BulkDelete(g, op->deleted_nodes, node_count, op->deleted_edges,
					 edge_count, &node_deleted, &relationships_deleted);

	if(op->stats) {
		op->stats->nodes_deleted += node_deleted;
		op->stats->relationships_deleted += relationships_deleted;
	}

cleanup:
	/* Release lock, no harm in trying to release an unlocked lock. */
	QueryCtx_UnlockCommit(&op->op);
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
				NULL, NULL, DeleteClone, DeleteFree, true, plan);

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
			op->deleted_nodes = array_append(op->deleted_nodes, *n);
		} else if(type & T_EDGE) {
			Edge *e = (Edge *)value.ptrval;
			op->deleted_edges = array_append(op->deleted_edges, *e);
		} else if(type & T_NULL) {
			continue; // Ignore null values.
		} else {
			/* Expression evaluated to a non-graph entity type
			 * clear pending deletions and raise an exception. */
			array_clear(op->deleted_nodes);
			array_clear(op->deleted_edges);
			// If evaluating the expression allocated any memory, free it.
			SIValue_Free(value);

			QueryCtx_SetError("Delete type mismatch, expecting either Node or Relationship.");
			QueryCtx_RaiseRuntimeException();
			break;
		}
	}

	return r;
}

static OpBase *DeleteClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_DELETE);
	OpDelete *op = (OpDelete *)opBase;
	AR_ExpNode **exps;
	array_clone_with_cb(exps, op->exps, AR_EXP_Clone);
	return NewDeleteOp(plan, exps);
}

static void DeleteFree(OpBase *ctx) {
	OpDelete *op = (OpDelete *)ctx;

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

