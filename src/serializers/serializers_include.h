/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

// A single header file including all important headers for serialization.

// Include Redis Modules API.
#include "../redismodule.h"
// Include Graph Context.
#include "../graph/graphcontext.h"
// Include Query contxt.
#include "../query_ctx.h"
// Include slow log.
#include "../slow_log/slow_log.h"
// Include GraphBLAS.
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
// Utils.
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../util/qsort.h"
// Non primitive data types.
#include "../datatypes/array.h"
