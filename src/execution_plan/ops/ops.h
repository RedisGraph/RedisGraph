/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

/* Include all available execution plan operations. */
#include "op_aggregate.h"
#include "op_all_node_scan.h"
#include "op_create.h"
#include "op_delete.h"
#include "op_filter.h"
#include "op_node_by_label_scan.h"
#include "op_index_scan.h"
#include "op_update.h"
#include "op_conditional_traverse.h"
#include "op_cartesian_product.h"
#include "op_merge.h"
#include "op_cond_var_len_traverse.h"
#include "op_unwind.h"
#include "op_sort.h"
#include "op_results.h"
#include "op_project.h"
#include "op_distinct.h"
#include "op_skip.h"
#include "op_limit.h"
#include "op_expand_into.h"
#include "op_node_by_id_seek.h"
#include "op_procedure_call.h"
#include "op_value_hash_join.h"
#include "op_apply.h"
#include "op_join.h"
#include "op_argument.h"
#include "op_merge_create.h"
#include "op_semi_apply.h"
#include "op_apply_multiplexer.h"
#include "op_optional.h"

