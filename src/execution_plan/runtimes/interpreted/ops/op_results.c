/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "RG.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"
#include "../../../../configuration/config.h"
#include "../../../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record ResultsConsume(RT_OpBase *opBase);
static RT_OpResult ResultsInit(RT_OpBase *opBase);

RT_OpBase *RT_NewResultsOp(const RT_ExecutionPlan *plan, const Results *op_desc) {
	RT_Results *op = rm_malloc(sizeof(RT_Results));
	op->op_desc = op_desc;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, ResultsInit,
		ResultsConsume, NULL, NULL, plan);

	return (RT_OpBase *)op;
}

static RT_OpResult ResultsInit(RT_OpBase *opBase) {
	RT_Results *op = (RT_Results *)opBase;
	op->result_set = QueryCtx_GetResultSet();
	Config_Option_get(Config_RESULTSET_MAX_SIZE, &op->result_set_size_limit);
	return OP_OK;
}

/* Results consume operation
 * called each time a new result record is required */
static Record ResultsConsume(RT_OpBase *opBase) {
	Record r = NULL;
	RT_Results *op = (RT_Results *)opBase;

	// enforce result-set size limit
	if(op->result_set_size_limit == 0) return NULL;
	op->result_set_size_limit--;

	RT_OpBase *child = op->op.children[0];
	r = RT_OpBase_Consume(child);
	if(!r) return NULL;

	// append to final result set
	ResultSet_AddRecord(op->result_set, r);
	return r;
}
