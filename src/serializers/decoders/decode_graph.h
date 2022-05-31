/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../serializers_include.h"

// load graph from pipe
GraphContext *PipeLoadGraph
(
	Pipe *p
);

// load graph from RDB
GraphContext *RdbLoadGraph
(
	RedisModuleIO *rdb
);

