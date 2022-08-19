/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "resultset.h"

// creates a new streaming result-set
// provides caller with a UID which can be used to reference returned result-set
ResultSet New_StreamingResultSet
(
	RedisModuleCtx *ctx,            // redis module context
	ResultSetFormatterType format,  // resultset format
	char **uid                      // result-set ID
);

