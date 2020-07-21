/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../ops/ops.h"
#include "../execution_plan.h"

/* The fullScantoLabelScan optimization searches for
 * AllNodeScan operations that have a label filter, such as
 * MATCH (a) WHERE a:L RETURN a
 * The AllNodeScan is converted into a Label Scan utilizing the appropriate label,
 * and the now-redundant filter is dropped. */
void fullScantoLabelScan(ExecutionPlan *plan);

