/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "stdlib.h"
#include "stdbool.h"

// Represent a graph encoding phase.
typedef enum {
	RESET,
	NODES,
	EDGES,
	DELETED_NODES,
	DELETED_EDGES,
	INDICES
} EncodePhase;

// A struct that maintains the state of a graph encoding to RDB or encode from RDB.
typedef struct {
	size_t keys_count;       // The number of keys representing the graph.
	size_t keys_processed;   // Count the number of procssed graph keys.
	EncodePhase phase;              // Represents the items currently encoded.
} GraphEncodeContext;

// Creates a new graph encoding context.
GraphEncodeContext *GraphEncodeContext_New(size_t key_count);

// Resest a graph encoding context.
void GraphEncodeContext_Reset(GraphEncodeContext *ctx);

// Retrive a graph encoding context encoding phase.
EncodePhase GraphEncodeContext_GetEncodePhase(GraphEncodeContext *ctx);

// Sets a graph encoding context phase.
void GraphEncodeContext_SetEncodePhase(GraphEncodeContext *ctx, EncodePhase phase);

// Retrive graph encoding context keys count.
size_t GraphEncodeContext_GetKeyCount(GraphEncodeContext *ctx);

// Retrive graph encoding context proccessed key count.
size_t GraphEncodeContext_GetProccessedKeyCount(GraphEncodeContext *ctx);

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphEncodeContext_Finished(GraphEncodeContext *ctx);

// Increases the number of keys representing the graph, by a given delta.
void GraphEncodeContext_IncreaseKeyCount(GraphEncodeContext *ctx, size_t delta);

// Decrease the number of key representing the graph, by a given delta.
void GraphEncodeContext_DecreaseKeyCount(GraphEncodeContext *ctx, size_t delta);

// Increases the number of processed graph keys.
void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx);

// Free graph encoding context.
void GraphEncodeContext_Free(GraphEncodeContext *ctx);
