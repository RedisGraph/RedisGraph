/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OPTIMIZATIONS_H__
#define __OPTIMIZATIONS_H__

#include "./apply_join.h"
#include "./apply_skip.h"
#include "./apply_limit.h"
#include "./seek_by_id.h"
#include "./reduce_count.h"
#include "./reduce_scans.h"
#include "./reduce_filters.h"
#include "./traverse_order.h"
#include "./compact_filters.h"
#include "./utilize_indices.h"
#include "./reduce_distinct.h"
#include "./reduce_traversal.h"
#include "./optimize_cartesian_product.h"
#include "./filter_variable_length_edges.h"

#endif

