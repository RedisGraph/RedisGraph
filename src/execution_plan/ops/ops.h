/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

// Include all available execution plan operations
#include "execution_plan/ops/op_aggregate.h"
#include "execution_plan/ops/op_all_node_scan.h"
#include "execution_plan/ops/op_create.h"
#include "execution_plan/ops/op_delete.h"
#include "execution_plan/ops/op_filter.h"
#include "execution_plan/ops/op_node_by_label_scan.h"
#include "execution_plan/ops/op_index_scan.h"
#include "execution_plan/ops/op_produce_results.h"
#include "execution_plan/ops/op_update.h"
#include "execution_plan/ops/op_traverse.h"
#include "execution_plan/ops/op_conditional_traverse.h"
#include "execution_plan/ops/op_cartesian_product.h"
#include "execution_plan/ops/op_merge.h"
#include "execution_plan/ops/op_cond_var_len_traverse.h"
#include "execution_plan/ops/op_unwind.h"
#include "execution_plan/ops/op_sort.h"
#include "execution_plan/ops/op_project.h"
#include "execution_plan/ops/op_distinct.h"
