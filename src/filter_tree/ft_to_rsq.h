/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "filter_tree.h"
#include "../../deps/RediSearch/src/redisearch_api.h"

// construct a RediSearch query node from filter tree
RSQNode *_filterTreeToQueryNode(FT_FilterNode *filter, RSIndex *sp);

