/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "stdlib.h"
#include "stdbool.h"

// A struct that maintains the state of a graph decoding from RDB.
typedef struct {
	bool graph_decoding;        // Indication if the current graph is currently in decoding process.
	uint64_t keys_processed;    // Count the number of procssed graph keys.
} GraphDecodeContext;

// Creates a new graph decoding context.
GraphDecodeContext *GraphDecodeContext_New();

// Resest a graph decoding context.
void GraphDecodeContext_Reset(GraphDecodeContext *ctx);

// Sets the decoding context to indicate about decoding in progress.
void GraphDecodeContext_SetGraphInDecode(GraphDecodeContext *ctx);

// Return if there is  decoding in progress.
bool GraphDecodeContext_IsGraphInDecode(GraphDecodeContext *ctx);

// Retrive graph decoding context proccessed key count.
uint64_t GraphDecodeContext_GetProccessedKeyCount(const GraphDecodeContext *ctx);

// Increases the number of processed graph keys.
void GraphDecodeContext_IncreaseProcessedCount(GraphDecodeContext *ctx);

// Free graph decoding context.
void GraphDecodeContext_Free(GraphDecodeContext *ctx);
