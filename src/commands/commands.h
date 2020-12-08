/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "cmd_query.h"
#include "cmd_delete.h"
#include "cmd_config.h"
#include "cmd_explain.h"
#include "cmd_profile.h"
#include "cmd_slowlog.h"
#include "cmd_dispatcher.h"
#include "cmd_bulk_insert.h"

//------------------------------------------------------------------------------
// Module Commands
//------------------------------------------------------------------------------

typedef enum {
	CMD_UNKNOWN        = 0,
	CMD_QUERY          = 1,
	CMD_RO_QUERY       = 2,
	CMD_DELETE         = 3,
	CMD_CONFIG         = 4,
	CMD_EXPLAIN        = 5,
	CMD_PROFILE        = 6,
	CMD_BULK_INSERT    = 7,
	CMD_SLOWLOG        = 8
} GRAPH_Commands;

