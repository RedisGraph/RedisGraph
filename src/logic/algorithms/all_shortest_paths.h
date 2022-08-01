/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "all_paths.h"

int AllShortestPaths_FindMinimumLength(
	AllPathsCtx *ctx,  // shortest path context
	Node *src,         // start traversing from `src`
	Node *dest         // end traversing when `dest` is reached
);

// get next shortest path
// returns NULL when all shortest paths from `src` to `dest`
// had been consumed
Path *AllShortestPaths_NextPath
(
	AllPathsCtx *ctx
);

