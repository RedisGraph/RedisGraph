/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __REDUCE_SCANS_H__
#define __REDUCE_SCANS_H__

#include "../execution_plan.h"

/* The reduce scans optimizer searches the execution plans for
 * SCAN operations which set node N, in-case there's an earlier
 * operation within the execution plan e.g. PROCEDURE-CALL which sets N
 * then omit SCAN. */
void reduceScans(ExecutionPlan *plan);

#endif
