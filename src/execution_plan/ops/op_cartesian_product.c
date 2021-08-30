/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "op_cartesian_product.h"

OpBase *NewCartesianProductOp(const ExecutionPlan *plan) {
	CartesianProduct *op = rm_malloc(sizeof(CartesianProduct));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CARTESIAN_PRODUCT, "Cartesian Product",
		NULL, false, plan);
		
	return (OpBase *)op;
}
