/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_index_scan.h"
#include "../../query_ctx.h"
#include "shared/print_functions.h"
#include "../../filter_tree/ft_to_rsq.h"

// forward declarations
static OpResult IndexScanInit(OpBase *opBase);
static Record IndexScanConsume(OpBase *opBase);
static Record IndexScanConsumeFromChild(OpBase *opBase);
static OpResult IndexScanReset(OpBase *opBase);
static void IndexScanFree(OpBase *opBase);

static int IndexScanToString(const OpBase *ctx, char *buf, uint buf_len) {
	IndexScan *op = (IndexScan *)ctx;
	return ScanToString(ctx, buf, buf_len, op->n.alias, op->n.label);
}

OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, NodeScanCtx n,
		RSIndex *idx, FT_FilterNode *filter) {
	// validate inputs
	ASSERT(g != NULL);
	ASSERT(idx != NULL);
	ASSERT(plan != NULL);
	ASSERT(filter != NULL);

	IndexScan *op = rm_malloc(sizeof(IndexScan));
	op->g                    =  g;
	op->n                    =  n;
	op->idx                  =  idx;
	op->iter                 =  NULL;
	op->filter               =  filter;
	op->child_record         =  NULL;
	op->unresolved_filters   =  NULL;
	op->rebuild_index_query  =  false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_INDEX_SCAN, "Index Scan", IndexScanInit, IndexScanConsume,
				IndexScanReset, IndexScanToString, NULL, IndexScanFree, false, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n.alias);
	return (OpBase *)op;
}

static OpResult IndexScanInit(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;

	if(opBase->childCount > 0) {
		// find out how many different entities are refered to 
		// within the filter tree, if number of entities equals 1
		// (current node being scanned) there's no need to re-build the index
		// query for every input record, construct it once now
		rax *entities = FilterTree_CollectModified(op->filter);
		op->rebuild_index_query = raxSize(entities) > 1; // this is us
		raxFree(entities);

		OpBase_UpdateConsume(opBase, IndexScanConsumeFromChild);
	}

	// resolve label ID now if it is still unknown
	if(op->n.label_id == GRAPH_UNKNOWN_LABEL) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Schema *schema = GraphContext_GetSchema(gc, op->n.label, SCHEMA_NODE);
		ASSERT(schema != NULL);
		op->n.label_id = schema->id;
	}

	return OP_OK;
}

static inline void _UpdateRecord(IndexScan *op, Record r, EntityID node_id) {
	// Populate the Record with the graph entity data.
	Node n = GE_NEW_LABELED_NODE(op->n.label, op->n.label_id);
	int res = Graph_GetNode(op->g, node_id, &n);
	ASSERT(res != 0);
	Record_AddNode(r, op->nodeRecIdx, n);
}

static inline bool _PassUnresolvedFilters(const IndexScan *op, Record r) {
	bool           res                  =  true;
	FT_FilterNode  *unresolved_filters  =  op->unresolved_filters;

	if(unresolved_filters != NULL) {
		res = (FilterTree_applyFilters(unresolved_filters, r) == FILTER_PASS);
	}

	return res;
}

static Record IndexScanConsumeFromChild(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	const EntityID *nodeId = NULL;

pull_index:
	//--------------------------------------------------------------------------
	// pull from index
	//--------------------------------------------------------------------------

	if(op->iter != NULL) {
		nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
		if(nodeId != NULL) {
			// populate record with node
			_UpdateRecord(op, op->child_record, *nodeId);
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

		// FilterTreeToQueryNode will try to convert 'filter' into a RediSearch
		// query, there are 3 different outcomes:
		// 1. entier filter is converted
		//    in which case rs_query_node is set and unresolved_filters is NULL
		// 2. a part of the filter is converted
		//    in which case both rs_query_node and unresolved_filters are set
		// 3. index can't be utilized
		//    in which case rs_query_node is NULL and unresolved_filters is set
		RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters,
				filter, op->idx);
		FilterTree_Free(filter);

		// can not have both RediSearch query and unresolved filters set to NULL
		ASSERT(rs_query_node != NULL || op->unresolved_filters != NULL);

		// cann't utilize the index
		if(rs_query_node == NULL) {
			// apply unresolved filters to input record
			if(FilterTree_applyFilters(op->unresolved_filters,
					op->child_record) == FILTER_PASS) return op->child_record;
		}

		// create iterator only if index can be utilized
		if(rs_query_node != NULL) {
			op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
		}
	} else {
		if(op->iter == NULL) {
			// first call to consume, create query and iterator
			RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters, op->filter, op->idx);
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

static Record IndexScanConsume(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;

	// create iterator on first call
	if(op->iter == NULL) {
		RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters,
				op->filter, op->idx);
		ASSERT(op->unresolved_filters == NULL);

		op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
	}

	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx,
			NULL);
	if(!nodeId) return NULL;

	// populate the Record with the actual node
	Record r = OpBase_CreateRecord((OpBase *)op);
	_UpdateRecord(op, r, *nodeId);

	return r;
}

static OpResult IndexScanReset(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;

	if(op->rebuild_index_query) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	} else {
		RediSearch_ResultsIteratorReset(op->iter);
	}

	return OP_OK;
}

static void IndexScanFree(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	/* As long as this Index iterator is alive the index is
	 * read locked, if this index scan operation is part of
	 * a query which will modified this index we'll be stuck in
	 * a dead lock, as we're unable to acquire index write lock. */
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

