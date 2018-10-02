/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_CARTESIANPRODUCT_H__
#define __OP_CARTESIANPRODUCT_H__

#include "op.h"

/* Cartesian product AKA Join. 
 * Currently this operation is a NOP 
 * once execution plan refactoring will take place 
 * we'll implement its logic. */
 typedef struct {
     OpBase op;
     bool init;
 } CartesianProduct;

OpBase* NewCartesianProductOp();
OpResult CartesianProductConsume(OpBase *opBase, Record *r);
OpResult CartesianProductReset(OpBase *opBase);
void CartesianProductFree(OpBase *opBase);

#endif