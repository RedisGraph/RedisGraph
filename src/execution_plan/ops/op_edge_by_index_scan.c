/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_edge_by_index_scan.h"
#include "../../query_ctx.h"
#include "shared/print_functions.h"
#include "../../filter_tree/ft_to_rsq.h"

// forward declarations
static OpResult EdgeIndexScanInit(OpBase *opBase);
static Record EdgeIndexScanConsume(OpBase *opBase);
static Record EdgeIndexScanConsumeFromChild(OpBase *opBase);
static OpResult EdgeIndexScanReset(OpBase *opBase);
static void EdgeIndexScanFree(OpBase *opBase);

static void EdgeIndexScanToString
(
	const OpBase *ctx,
	sds *buf
) {
	ASSERT(ctx != NULL);
	ASSERT(buf != NULL);

	OpEdgeIndexScan *op = (OpEdgeIndexScan *)ctx;

	*buf = sdscatprintf(*buf, "%s | ", ctx->name);
	QGEdge_ToString(op->edge, buf);
}

OpBase *NewEdgeIndexScanOp
(
	const ExecutionPlan *plan,
	Graph *g,
	QGEdge *e,
	RSIndex *idx,
	FT_FilterNode *filter
) {
	// validate inputs
	ASSERT(g                       != NULL);
	ASSERT(e                       != NULL);
	ASSERT(idx                     != NULL);
	ASSERT(plan                    != NULL);
	ASSERT(filter                  != NULL);
	ASSERT(QGEdge_Alias(e)         != NULL);
	ASSERT(QGEdge_RelationCount(e) == 1);

	OpEdgeIndexScan *op      =  rm_malloc(sizeof(OpEdgeIndexScan));
	op->g                    =  g;
	op->idx                  =  idx;
	op->edge                 =  e;
	op->iter                 =  NULL;
	op->filter               =  filter;
	op->child_record         =  NULL;
	op->unresolved_filters   =  NULL;
	op->rebuild_index_query  =  false;

	// set our Op operations
	OpBase_Init(
			(OpBase *)op,
			OPType_EDGE_BY_INDEX_SCAN,
			"Edge By Index Scan",
			EdgeIndexScanInit,
			EdgeIndexScanConsume,
			EdgeIndexScanReset,
			EdgeIndexScanToString,
			NULL,
			EdgeIndexScanFree,
			false,
			plan);

	op->edgeRecIdx = OpBase_Modifies((OpBase *)op, QGEdge_Alias(e));

	const char *src_alias = QGNode_Alias(QGEdge_Src(e));
	op->srcAware = OpBase_Aware((OpBase *)op, src_alias, &op->srcRecIdx);
	if(!op->srcAware) {
		op->srcRecIdx = OpBase_Modifies((OpBase *)op, QGNode_Alias(QGEdge_Src(e)));
	}

	const char *dest_alias = QGNode_Alias(QGEdge_Dest(e));
	op->destAware = OpBase_Aware((OpBase *)op, dest_alias, &op->destRecIdx);
	if(!op->destAware) {
		op->destRecIdx = OpBase_Modifies((OpBase *)op, dest_alias);
	}

	return (OpBase *)op;
}

static OpResult EdgeIndexScanInit
(
	OpBase *opBase
) {
	OpEdgeIndexScan *op = (OpEdgeIndexScan *)opBase;

	if(opBase->childCount > 0) {
		// find out how many different entities are refered to 
		// within the filter tree, if number of entities equals 1
		// (current edge being scanned) there's no need to re-build the index
		// query for every input record
		rax *entities = FilterTree_CollectModified(op->filter);
		op->rebuild_index_query = raxSize(entities) > 1;
		raxFree(entities);

		OpBase_UpdateConsume(opBase, EdgeIndexScanConsumeFromChild);
	}

	// resolve relation ID now if it is still unknown
	if(QGEdge_RelationID(op->edge, 0) == GRAPH_UNKNOWN_RELATION) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Schema *s = GraphContext_GetSchema(gc, QGEdge_Relation(op->edge, 0),
				SCHEMA_EDGE);
		ASSERT(s != NULL);

		op->edge->reltypeIDs[0] = Schema_ID(s);
	}

	return OP_OK;
}

static inline void _UpdateRecord
(
	OpEdgeIndexScan *op,
	Record r,
	const EdgeIndexKey *edge_key
) {
	// populate Record with edge, src and destination node
	int res;

	Edge  *e     =  Record_GetEdge(r,  op->edgeRecIdx);
	Node  *src   =  Record_GetNode(r,  op->srcRecIdx);
	Node  *dest  =  Record_GetNode(r,  op->destRecIdx);

	EntityID  src_id   =  edge_key->src_id;
	EntityID  dest_id  =  edge_key->dest_id;
	EntityID  edge_id  =  edge_key->edge_id;

	res = Graph_GetNode(op->g, src_id, src);
	ASSERT(res != 0);

	res = Graph_GetNode(op->g, dest_id, dest);
	ASSERT(res != 0);

	res = Graph_GetEdge(op->g, edge_id, e);
	ASSERT(res != 0);

	Edge_SetSrcNode(e, src);
	Edge_SetDestNode(e, dest);
	Edge_SetRelationID(e, QGEdge_RelationID(op->edge, 0));
}

static inline bool _PassUnresolvedFilters
(
	const OpEdgeIndexScan *op,
	Record r
) {
	FT_FilterNode *unresolved_filters = op->unresolved_filters;
	if(unresolved_filters == NULL) return true; // no filters

	return FilterTree_applyFilters(unresolved_filters, r) == FILTER_PASS;
}

static Record EdgeIndexScanConsumeFromChild
(
	OpBase *opBase
) {
	OpEdgeIndexScan	*op = (OpEdgeIndexScan*)opBase;
	const EdgeIndexKey *edgeKey = NULL;

pull_index:
	//--------------------------------------------------------------------------
	// pull from index
	//--------------------------------------------------------------------------

	if(op->iter != NULL) {
		while((edgeKey = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL))
				!= NULL) {
			// populate record with edge
			_UpdateRecord(op, op->child_record, edgeKey);
			// apply unresolved filters
			if(_PassUnresolvedFilters(op, op->child_record)) {
				// clone the held Record, as it will be freed upstream
				return OpBase_CloneRecord(op->child_record);
			}
		}
	}

	//--------------------------------------------------------------------------
	// index depleted
	//--------------------------------------------------------------------------

	// free input record
	if(op->child_record != NULL) {
		OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}

	//--------------------------------------------------------------------------
	// pull from child
	//--------------------------------------------------------------------------

	op->child_record = OpBase_Consume(op->op.children[0]);
	if(op->child_record == NULL) return NULL; // depleted

	//--------------------------------------------------------------------------
	// reset index iterator
	//--------------------------------------------------------------------------

	if(op->rebuild_index_query) {
		// free previous iterator
		if(op->iter != NULL) {
			RediSearch_ResultsIteratorFree(op->iter);
			op->iter = NULL;
		}

		// free previous unresolved filters
		if(op->unresolved_filters != NULL) {
			FilterTree_Free(op->unresolved_filters);
			op->unresolved_filters = NULL;
		}

		// rebuild index query, probably relies on runtime values
		// resolve runtime variables within filter
		FT_FilterNode *filter = FilterTree_Clone(op->filter);
		FilterTree_ResolveVariables(filter, op->child_record);

		// make sure there's only one unresolve entity in filter
		#ifdef RG_DEBUG
		{
			rax *entities = FilterTree_CollectModified(filter);
			ASSERT(raxSize(entities) == 1);
			raxFree(entities);
		}
		#endif

		// convert filter into a RediSearch query
		RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters,
				filter, op->idx);
		FilterTree_Free(filter);

		// if src =

		// create iterator
		ASSERT(rs_query_node != NULL);
		op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
	} else {
		// build index query only once (first call)
		// reset it if already initialized
		if(op->iter == NULL) {
			// first call to consume, create query and iterator
			RSQNode *rs_query_node = FilterTreeToQueryNode(
					&op->unresolved_filters, op->filter, op->idx);
			ASSERT(rs_query_node != NULL);
			ASSERT(op->unresolved_filters == NULL);
			op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
		} else {
			// reset existing iterator
			RediSearch_ResultsIteratorReset(op->iter);
		}
	}

	// repull from index
	goto pull_index;
}

static Record EdgeIndexScanConsume
(
	OpBase *opBase
) {
	OpEdgeIndexScan *op = (OpEdgeIndexScan *)opBase;

	// create iterator on first call
	if(op->iter == NULL) {
		RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters,
				op->filter, op->idx);
		ASSERT(op->unresolved_filters == NULL);

		op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
	}

	const EdgeIndexKey *edgeKey = RediSearch_ResultsIteratorNext(op->iter,
			op->idx, NULL);
	if(!edgeKey) return NULL;

	// populate the Record with the actual edge
	Record r = OpBase_CreateRecord((OpBase *)op);
	_UpdateRecord(op, r, edgeKey);

	return r;
}

static OpResult EdgeIndexScanReset(OpBase *opBase) {
	OpEdgeIndexScan *op = (OpEdgeIndexScan *)opBase;

	if(op->rebuild_index_query) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	} else {
		RediSearch_ResultsIteratorReset(op->iter);
	}

	return OP_OK;
}

static void EdgeIndexScanFree(OpBase *opBase) {
	OpEdgeIndexScan *op = (OpEdgeIndexScan *)opBase;
	// as long as this Index iterator is alive the index is
	// read locked, if this index scan operation is part of
	// a query which will modified this index we'll be stuck in
	// a dead lock, as we're unable to acquire index write lock
	if(op->iter) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	}

	if(op->child_record) {
		OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}

	if(op->filter) {
		FilterTree_Free(op->filter);
		op->filter = NULL;
	}

	if(op->unresolved_filters) {
		FilterTree_Free(op->unresolved_filters);
		op->unresolved_filters = NULL;
	}
}

