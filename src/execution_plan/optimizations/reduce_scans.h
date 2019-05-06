/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __REDUCE_SCANS_H__
#define __REDUCE_SCANS_H__

#include "../execution_plan.h"

/* The reduce scans optimizer searches the execution plans for 
 * consecutive traversal and scan operations, in such cases
 * performing SCAN, will only slow us down, and so this optimization
 * will remove such SCAN operations. */

/* TODO: Once we'll have statistics regarding the number of different types
 * using a relation we'll be able to drop SCAN and additional typed matrix 
 * multiplication. */
void reduceScans(ExecutionPlanSegment *plan);

#endif
