/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op.h"
#include "ops.h"
#include "RG.h"
#include "../runtime_execution_plan.h"
#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"
#include "../../../../util/simple_timer.h"

static RT_fpNew new_funcs[] = {
	(RT_fpNew)RT_NewAllNodeScanOp,           // OPType_ALL_NODE_SCAN
	(RT_fpNew)RT_NewNodeByLabelScanOp,       // OPType_NODE_BY_LABEL_SCAN
	(RT_fpNew)RT_NewIndexScanOp,             // OPType_INDEX_SCAN
	(RT_fpNew)RT_NewNodeByIdSeekOp,          // OPType_NODE_BY_ID_SEEK
	(RT_fpNew)RT_NewNodeByLabelScanOp,       // OPType_NODE_BY_LABEL_AND_ID_SCAN
	(RT_fpNew)RT_NewExpandIntoOp,            // OPType_EXPAND_INTO
	(RT_fpNew)RT_NewCondTraverseOp,          // OPType_CONDITIONAL_TRAVERSE
	(RT_fpNew)RT_NewCondVarLenTraverseOp,    // OPType_CONDITIONAL_VAR_LEN_TRAVERSE
	(RT_fpNew)RT_NewCondVarLenTraverseOp,    // OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO
	(RT_fpNew)RT_NewResultsOp,               // OPType_RESULTS
	(RT_fpNew)RT_NewProjectOp,               // OPType_PROJECT
	(RT_fpNew)RT_NewAggregateOp,             // OPType_AGGREGATE
	(RT_fpNew)RT_NewSortOp,                  // OPType_SORT
	(RT_fpNew)RT_NewSkipOp,                  // OPType_SKIP
	(RT_fpNew)RT_NewLimitOp,                 // OPType_LIMIT
	(RT_fpNew)RT_NewDistinctOp,              // OPType_DISTINCT
	(RT_fpNew)RT_NewMergeOp,                 // OPType_MERGE
	(RT_fpNew)RT_NewMergeCreateOp,           // OPType_MERGE_CREATE
	(RT_fpNew)RT_NewFilterOp,                // OPType_FILTER
	(RT_fpNew)RT_NewCreateOp,                // OPType_CREATE
	(RT_fpNew)RT_NewUpdateOp,                // OPType_UPDATE
	(RT_fpNew)RT_NewDeleteOp,                // OPType_DELETE
	(RT_fpNew)RT_NewUnwindOp,                // OPType_UNWIND
	(RT_fpNew)RT_NewProcCallOp,              // OPType_PROC_CALL
	(RT_fpNew)RT_NewArgumentOp,              // OPType_ARGUMENT
	(RT_fpNew)RT_NewCartesianProductOp,      // OPType_CARTESIAN_PRODUCT
	(RT_fpNew)RT_NewValueHashJoin,           // OPType_VALUE_HASH_JOIN
	(RT_fpNew)RT_NewApplyOp,                 // OPType_APPLY
	(RT_fpNew)RT_NewJoinOp,                  // OPType_JOIN
	(RT_fpNew)RT_NewSemiApplyOp,             // OPType_SEMI_APPLY
	(RT_fpNew)RT_NewSemiApplyOp,             // OPType_ANTI_SEMI_APPLY
	(RT_fpNew)RT_NewApplyMultiplexerOp,      // OPType_OR_APPLY_MULTIPLEXER
	(RT_fpNew)RT_NewApplyMultiplexerOp,      // OPType_AND_APPLY_MULTIPLEXER
	(RT_fpNew)RT_NewOptionalOp               // OPType_OPTIONAL
};

void RT_OpBase_Init(RT_OpBase *op, const OpBase *op_desc, RT_fpToString toString, 
	RT_fpInit init, RT_fpConsume consume, RT_fpReset reset, RT_fpFree free,
	const struct RT_ExecutionPlan *plan) {

	op->op_desc = op_desc;
	op->plan = plan;
	op->parent = NULL;
	op->childCount = 0;
	op->children = NULL;
	op->parent = NULL;
	op->stats = NULL;
	op->op_initialized = false;
	op->modifies = NULL;

	// Function pointers.
	op->init = init;
	op->consume = consume;
	op->reset = reset;
	op->free = free;
	op->toString = toString;

	op->profile = NULL;
}

inline Record RT_OpBase_Consume(RT_OpBase *op) {
	return op->consume(op);
}

bool RT_OpBase_Aware(RT_OpBase *op, const char *alias, uint *idx) {
	rax *mapping = RT_ExecutionPlan_GetMappings(op->plan);
	void *rec_idx = raxFind(mapping, (unsigned char *)alias, strlen(alias));
	if(idx) *idx = (uintptr_t)rec_idx;
	return (rec_idx != raxNotFound);
}

void RT_OpBase_PropagateFree(RT_OpBase *op) {
	if(op->free) op->free(op);
	for(int i = 0; i < op->childCount; i++) RT_OpBase_PropagateFree(op->children[i]);
}

void RT_OpBase_PropagateReset(RT_OpBase *op) {
	if(op->reset) {
		RT_OpResult res = op->reset(op);
		ASSERT(res == OP_OK);
	}
	for(int i = 0; i < op->childCount; i++) RT_OpBase_PropagateReset(op->children[i]);
}

static void _OpBase_StatsToString(const RT_OpBase *op, sds *buff) {
	*buff = sdscatprintf(*buff,
					" | Records produced: %d, Execution time: %f ms",
					op->stats->profileRecordCount,
					op->stats->profileExecTime);
}

Record RT_OpBase_Profile(RT_OpBase *op) {
	double tic [2];
	// Start timer.
	simple_tic(tic);
	Record r = op->profile(op);
	// Stop timer and accumulate.
	op->stats->profileExecTime += simple_toc(tic);
	if(r) op->stats->profileRecordCount++;
	return r;
}

void RT_OpBase_ToString(const RT_OpBase *op, sds *buff) {
	int bytes_written = 0;

	if(op->toString) op->toString(op, buff);
	else *buff = sdscatprintf(*buff, "%s", op->op_desc->name);

	if(op->stats) _OpBase_StatsToString(op, buff);
}

bool RT_OpBase_IsWriter(RT_OpBase *op) {
	return op->op_desc->writer;
}

void RT_OpBase_UpdateConsume(RT_OpBase *op, RT_fpConsume consume) {
	ASSERT(op != NULL);
	/* If Operation is profiled, update profiled function.
	 * otherwise update consume function. */
	if(op->profile != NULL) op->profile = consume;
	else op->consume = consume;
}

inline Record RT_OpBase_CreateRecord(const RT_OpBase *op) {
	return RT_ExecutionPlan_BorrowRecord((struct RT_ExecutionPlan *)op->plan);
}

Record RT_OpBase_CloneRecord(Record r) {
	Record clone = RT_ExecutionPlan_BorrowRecord((struct RT_ExecutionPlan *)r->owner);
	Record_Clone(r, clone);
	return clone;
}

inline OPType RT_OpBase_Type(const RT_OpBase *op) {
	ASSERT(op != NULL);
	return op->op_desc->type;
}

inline void RT_OpBase_DeleteRecord(Record r) {
	RT_ExecutionPlan_ReturnRecord(r->owner, r);
}

RT_OpBase *RT_OpBase_New(const struct RT_ExecutionPlan *plan, const OpBase *op_desc) {
	return new_funcs[op_desc->type](plan, op_desc);
}

RT_OpBase *RT_OpBase_Clone(const struct RT_ExecutionPlan *plan, const RT_OpBase *op) {
	return new_funcs[op->op_desc->type](plan, op->op_desc);
}

void RT_OpBase_Free(RT_OpBase *op) {
	// Free internal operation
	if(op->free) op->free(op);
	if(op->children) rm_free(op->children);
	if(op->modifies) array_free(op->modifies);
	if(op->stats) rm_free(op->stats);
	rm_free(op);
}
