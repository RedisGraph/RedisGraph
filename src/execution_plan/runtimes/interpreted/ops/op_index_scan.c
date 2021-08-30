/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_index_scan.h"
#include "../../../../query_ctx.h"
#include "../../../ops/shared/print_functions.h"
#include "../../../../filter_tree/ft_to_rsq.h"

// forward declarations
static RT_OpResult IndexScanInit(RT_OpBase *opBase);
static Record IndexScanConsume(RT_OpBase *opBase);
static Record IndexScanConsumeFromChild(RT_OpBase *opBase);
static RT_OpResult IndexScanReset(RT_OpBase *opBase);
static void IndexScanFree(RT_OpBase *opBase);

static void IndexScanToString(const RT_OpBase *ctx, sds *buf) {
	RT_IndexScan *op = (RT_IndexScan *)ctx;
	return ScanToString(ctx, buf, op->op_desc->n.alias, op->op_desc->n.label);
}

RT_OpBase *RT_NewIndexScanOp(const RT_ExecutionPlan *plan, const IndexScan *op_desc) {
	// validate inputs
	ASSERT(plan   != NULL);

	RT_IndexScan *op = rm_malloc(sizeof(RT_IndexScan));
	op->op_desc              =  op_desc;
	op->idx                  =  op_desc->idx;
	op->iter                 =  NULL;
	op->child_record         =  NULL;
	op->unresolved_filters   =  NULL;
	op->rebuild_index_query  =  false;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op,
		IndexScanToString, IndexScanInit, IndexScanConsume, IndexScanReset,
		IndexScanFree, plan);

	bool aware = RT_OpBase_Aware((RT_OpBase *)op, op_desc->n.alias, &op->nodeRecIdx);
	UNUSED(aware);
	ASSERT(aware);

	return (RT_OpBase *)op;
}

static RT_OpResult IndexScanInit(RT_OpBase *opBase) {
	RT_IndexScan *op = (RT_IndexScan *)opBase;
	op->g            =  QueryCtx_GetGraph();

	if(opBase->childCount > 0) {
		// find out how many different entities are refered to 
		// within the filter tree, if number of entities equals 1
		// (current node being scanned) there's no need to re-build the index
		// query for every input record
		rax *entities = FilterTree_CollectModified(op->op_desc->filter);
		op->rebuild_index_query = raxSize(entities) > 1; // this is us
		raxFree(entities);

		RT_OpBase_UpdateConsume(opBase, IndexScanConsumeFromChild);
	}

	return OP_OK;
}

static inline void _UpdateRecord(RT_IndexScan *op, Record r, EntityID node_id) {
	// Populate the Record with the graph entity data.
	Node n = GE_NEW_LABELED_NODE(op->op_desc->n.label, op->op_desc->n.label_id);
	int res = Graph_GetNode(op->g, node_id, &n);
	ASSERT(res != 0);
	Record_AddNode(r, op->nodeRecIdx, n);
}

static inline bool _PassUnresolvedFilters(const RT_IndexScan *op, Record r) {
	FT_FilterNode *unresolved_filters = op->unresolved_filters;
	if(unresolved_filters == NULL) return true; // no filters
	return FilterTree_applyFilters(unresolved_filters, r) == FILTER_PASS;
}

static Record IndexScanConsumeFromChild(RT_OpBase *opBase) {
	RT_IndexScan *op = (RT_IndexScan *)opBase;
	const EntityID *nodeId = NULL;

pull_index:
	//--------------------------------------------------------------------------
	// pull from index
	//--------------------------------------------------------------------------

	if(op->iter != NULL) {
		while((nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL))
				!= NULL) {
			// populate record with node
			_UpdateRecord(op, op->child_record, *nodeId);
			// apply unresolved filters
			if(_PassUnresolvedFilters(op, op->child_record)) {
				// clone the held Record, as it will be freed upstream
				return RT_OpBase_CloneRecord(op->child_record);
			}
		}
	}

	//--------------------------------------------------------------------------
	// index depleted
	//--------------------------------------------------------------------------

	// free input record
	if(op->child_record != NULL) {
		RT_OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}

	//--------------------------------------------------------------------------
	// pull from child
	//--------------------------------------------------------------------------

	op->child_record = RT_OpBase_Consume(op->op.children[0]);
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
		FT_FilterNode *filter = FilterTree_Clone(op->op_desc->filter);
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

		// create iterator
		ASSERT(rs_query_node != NULL);
		op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
	} else {
		// build index query only once (first call)
		// reset it if already initialized
		if(op->iter == NULL) {
			// first call to consume, create query and iterator
			RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters, op->op_desc->filter, op->idx);
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

static Record IndexScanConsume(RT_OpBase *opBase) {
	RT_IndexScan *op = (RT_IndexScan *)opBase;

	// create iterator on first call
	if(op->iter == NULL) {
		RSQNode *rs_query_node = FilterTreeToQueryNode(&op->unresolved_filters,
				op->op_desc->filter, op->idx);
		ASSERT(op->unresolved_filters == NULL);

		op->iter = RediSearch_GetResultsIterator(rs_query_node, op->idx);
	}

	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx,
			NULL);
	if(!nodeId) return NULL;

	// populate the Record with the actual node
	Record r = RT_OpBase_CreateRecord((RT_OpBase *)op);
	_UpdateRecord(op, r, *nodeId);

	return r;
}

static RT_OpResult IndexScanReset(RT_OpBase *opBase) {
	RT_IndexScan *op = (RT_IndexScan *)opBase;

	if(op->rebuild_index_query) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	} else {
		RediSearch_ResultsIteratorReset(op->iter);
	}

	return OP_OK;
}

static void IndexScanFree(RT_OpBase *opBase) {
	RT_IndexScan *op = (RT_IndexScan *)opBase;
	/* As long as this Index iterator is alive the index is
	 * read locked, if this index scan operation is part of
	 * a query which will modified this index we'll be stuck in
	 * a dead lock, as we're unable to acquire index write lock. */
	if(op->iter) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	}

	if(op->child_record) {
		RT_OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}

	if(op->unresolved_filters) {
		FilterTree_Free(op->unresolved_filters);
		op->unresolved_filters = NULL;
	}
}
