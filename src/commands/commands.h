/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
#include "cmd_slowlog.h"
#include "cmd_dispatcher.h"
#include "cmd_bulk_insert.h"

typedef enum {
	CMD_UNKNOWN,
	CMD_QUERY,
	CMD_RO_QUERY,
	CMD_DELETE,
	CMD_EXPLAIN,
	CMD_PROFILE,
	CMD_BULK_INSERT,
	CMD_SLOWLOG
} GRAPH_Commands;
