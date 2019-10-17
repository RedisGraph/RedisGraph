/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

//------------------------------------------------------------------------------
// Module Commands
//------------------------------------------------------------------------------
#include "cmd_query.h"
#include "cmd_delete.h"
#include "cmd_explain.h"
#include "cmd_profile.h"
#include "cmd_dispatcher.h"
#include "cmd_bulk_insert.h"

typedef enum {
    CMD_BULK_UNKNOWN,
	CMD_QUERY,
	CMD_DELETE,
	CMD_EXPLAIN,
    CMD_PROFILE,
    CMD_BULK_INSERT
} GRAPH_Commands;
