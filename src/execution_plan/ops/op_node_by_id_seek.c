/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"
#include "RG.h"
#include "shared/print_functions.h"
#include "../../query_ctx.h"

OpBase *NewNodeByIdSeekOp(const ExecutionPlan *plan, const char *alias, UnsignedRange *id_range) {

	NodeByIdSeek *op = rm_malloc(sizeof(NodeByIdSeek));
	op->alias = alias;

	op->minId = id_range->include_min ? id_range->min : id_range->min + 1;
	/* The largest possible entity ID is the same as Graph_RequiredMatrixDim.
	 * This value will be set on Init, to allow operation clone be independent
	 * on the current graph size.*/
	op->maxId = id_range->include_max ? id_range->max : id_range->max - 1;

	OpBase_Init((OpBase *)op, OPType_NODE_BY_ID_SEEK, "NodeByIdSeek", NULL,
		false, plan);

	OpBase_Modifies((OpBase *)op, alias);

	return (OpBase *)op;
}
