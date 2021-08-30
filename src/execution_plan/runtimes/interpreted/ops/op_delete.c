/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../../../errors.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"
#include "../../../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record DeleteConsume(RT_OpBase *opBase);
static RT_OpResult DeleteInit(RT_OpBase *opBase);
static void DeleteFree(RT_OpBase *opBase);

void _DeleteEntities(RT_OpDelete *op) {
	Graph  *g                     =  op->gc->g;
	uint   node_deleted           =  0;
	uint   edge_deleted           =  0;
	uint   implicit_edge_deleted  =  0;
	uint   node_count             =  array_len(op->deleted_nodes);
	uint   edge_count             =  array_len(op->deleted_edges);

	// nothing to delete, quickly return
	if((node_count + edge_count) == 0) goto cleanup;

	// lock everything
	QueryCtx_LockForCommit();

	if(GraphContext_HasIndices(op->gc)) {
		for(int i = 0; i < node_count; i++) {
			Node *n = op->deleted_nodes + i;
			GraphContext_DeleteNodeFromIndices(op->gc, n);
		}
	}

	if(edge_count <= EDGE_BULK_DELETE_THRESHOLD) {
		for(uint i = 0; i < edge_count; i++) {
			edge_deleted += Graph_DeleteEdge(g, op->deleted_edges + i);
		}
		edge_count = 0;
	}

	Graph_BulkDelete(g, op->deleted_nodes, node_count, op->deleted_edges,
					 edge_count, &node_deleted, &implicit_edge_deleted);

	if(op->stats != NULL) {
		op->stats->nodes_deleted          +=  node_deleted;
		op->stats->relationships_deleted  +=  edge_deleted;
		op->stats->relationships_deleted  +=  implicit_edge_deleted;
	}

cleanup:
	// release lock, no harm in trying to release an unlocked lock
	QueryCtx_UnlockCommit(&op->op);
}

RT_OpBase *RT_NewDeleteOp(const RT_ExecutionPlan *plan, const OpDelete *op_desc) {
	RT_OpDelete *op = rm_malloc(sizeof(RT_OpDelete));

	op->gc = QueryCtx_GetGraphCtx();
	op->op_desc = op_desc;
	op->stats = NULL;
	op->deleted_nodes = array_new(Node, 32);
	op->deleted_edges = array_new(Edge, 32);

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL,
		DeleteInit, DeleteConsume, NULL, DeleteFree, plan);

	return (RT_OpBase *)op;
}

static RT_OpResult DeleteInit(RT_OpBase *opBase) {
	RT_OpDelete *op = (RT_OpDelete *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	return OP_OK;
}

static Record DeleteConsume(RT_OpBase *opBase) {
	RT_OpDelete *op = (RT_OpDelete *)opBase;
	RT_OpBase *child = op->op.children[0];

	Record r = RT_OpBase_Consume(child);
	if(!r) return NULL;

	/* Expression should be evaluated to either a node or an edge
	 * which will be marked for deletion, if an expression is evaluated
	 * to a different value type e.g. Numeric a run-time expection is thrown. */
	for(int i = 0; i < op->op_desc->exp_count; i++) {
		AR_ExpNode *exp = op->op_desc->exps[i];
		SIValue value = AR_EXP_Evaluate(exp, r);
		SIType type = SI_TYPE(value);
		/* Enqueue entities for deletion. */
		if(type & T_NODE) {
			Node *n = (Node *)value.ptrval;
			array_append(op->deleted_nodes, *n);
		} else if(type & T_EDGE) {
			Edge *e = (Edge *)value.ptrval;
			array_append(op->deleted_edges, *e);
		} else if(type & T_NULL) {
			continue; // Ignore null values.
		} else {
			/* Expression evaluated to a non-graph entity type
			 * clear pending deletions and raise an exception. */
			array_clear(op->deleted_nodes);
			array_clear(op->deleted_edges);
			// If evaluating the expression allocated any memory, free it.
			SIValue_Free(value);

			ErrorCtx_RaiseRuntimeException("Delete type mismatch, expecting either Node or Relationship.");
			break;
		}
	}

	return r;
}

static void DeleteFree(RT_OpBase *ctx) {
	RT_OpDelete *op = (RT_OpDelete *)ctx;

	_DeleteEntities(op);

	if(op->deleted_nodes) {
		array_free(op->deleted_nodes);
		op->deleted_nodes = NULL;
	}

	if(op->deleted_edges) {
		array_free(op->deleted_edges);
		op->deleted_edges = NULL;
	}
}
