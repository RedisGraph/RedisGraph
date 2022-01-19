/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "all_paths.h"

int AllShortestPaths_FindMinimumLength(
	AllPathsCtx *ctx,
	Node *src,
	Node *dest
);

Path *AllShortestPaths_NextPath(AllPathsCtx *ctx);