/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"

/* Cartesian product AKA Join. */
typedef struct {
	OpBase op;
	bool init;
	Record r;
} CartesianProduct;

OpBase *NewCartesianProductOp(void);
