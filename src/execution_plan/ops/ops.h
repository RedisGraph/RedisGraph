/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OPS_H_
#define __OPS_H_

/* Include all available execution plan operations. */
#include "op_aggregate.h"
#include "op_all_node_scan.h"
#include "op_create.h"
#include "op_delete.h"
#include "op_filter.h"
#include "op_node_by_label_scan.h"
#include "op_index_scan.h"
#include "op_update.h"
#include "op_traverse.h"
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
#include "op_handoff.h"

#endif
