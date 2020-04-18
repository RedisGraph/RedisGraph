/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"

// A struct that maintains the state of a graph decoding from RDB.
typedef struct {
	bool graph_decoding;        // Indication if the current graph is currently in decoding process.
	uint64_t keys_processed;    // Count the number of procssed graph keys.
} GraphDecodeContext;

// Creates a new graph decoding context.
GraphDecodeContext *GraphDecodeContext_New();

// Reset a graph decoding context.
void GraphDecodeContext_Reset(GraphDecodeContext *ctx);

// Set the graph_decoding flag to true.
void GraphDecodeContext_SetGraphInDecode(GraphDecodeContext *ctx);

// Return true if decoding is in progress.
bool GraphDecodeContext_IsGraphInDecode(GraphDecodeContext *ctx);

// Get the number of keys decoded so far.
uint64_t GraphDecodeContext_GetProcessedKeyCount(const GraphDecodeContext *ctx);

// Increment the number of processed keys by one.
void GraphDecodeContext_IncreaseProcessedCount(GraphDecodeContext *ctx);

// Free graph decoding context.
void GraphDecodeContext_Free(GraphDecodeContext *ctx);
