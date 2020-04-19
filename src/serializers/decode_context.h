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
	uint64_t graph_keys_count;  // The number of keys representing the graph.
} GraphDecodeContext;

// Creates a new graph decoding context.
GraphDecodeContext *GraphDecodeContext_New();

// Reset a graph decoding context.
void GraphDecodeContext_Reset(GraphDecodeContext *ctx);

// Sets the number of keys required for decoding the graph.
void GraphDecodeContext_SetKeyCount(GraphDecodeContext *ctx, uint64_t key_count);

// Returns the number of keys required for decoding the graph.
uint64_t GraphDecodeContext_GetKeyCount(const GraphDecodeContext *ctx);

// Set the graph_decoding flag to true.
void GraphDecodeContext_SetGraphInDecode(GraphDecodeContext *ctx);

// Return true if decoding is in progress.
bool GraphDecodeContext_IsGraphInDecode(GraphDecodeContext *ctx);

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphDecodeContext_Finished(const GraphDecodeContext *ctx);

// Increment the number of processed keys by one.
void GraphDecodeContext_IncreaseProcessedCount(GraphDecodeContext *ctx);

// Free graph decoding context.
void GraphDecodeContext_Free(GraphDecodeContext *ctx);
