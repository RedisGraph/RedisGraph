#ifndef __OP_CARTESIANPRODUCT_H__
#define __OP_CARTESIANPRODUCT_H__

#include "op.h"

/* Cartesian product AKA Join. 
 * Currently this operation is a NOP 
 * once execution plan refactoring will take place 
 * we'll implement its logic. */
 typedef struct {
     OpBase op;
     int refresh;
 } CartesianProduct;

OpBase* NewCartesianProductOp();
CartesianProduct* NewCartesianProduct();
OpResult CartesianProductConsume(OpBase *opBase, QueryGraph* graph);
OpResult CartesianProductReset(OpBase *opBase);
void CartesianProductFree(OpBase *opBase);

#endif