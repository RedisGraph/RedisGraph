/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "stdlib.h"
#include "stdbool.h"
#include "../util/datablock/datablock.h"

// Represent a graph encoding phase.
typedef enum {
	RESET,
	NODES,
	DELETED_NODES,
	EDGES,
	DELETED_EDGES,
	GRAPH_SCHEMA
} EncodePhase;

// A struct that maintains the state of a graph encoding to RDB or encode from RDB.
typedef struct {
	uint64_t keys_count;                    // The number of keys representing the graph.
	uint64_t keys_processed;                // Count the number of procssed graph keys.
	EncodePhase phase;                      // Represents the items currently encoded.
	uint64_t processed_nodes;               // Number of encoded nodes.
	uint64_t processed_deleted_nodes;       // Number of encoded deleted nodes.
	uint64_t processed_edges;               // Number of encoded edges.
	uint64_t processed_deleted_edges;       // Number of encoded deleted edges.
	DataBlockIterator *datablock_iterator;  // Datablock iterator to be saved in the context.
} GraphEncodeContext;

// Creates a new graph encoding context.
GraphEncodeContext *GraphEncodeContext_New(uint64_t key_count);

// Resest a graph encoding context.
void GraphEncodeContext_Reset(GraphEncodeContext *ctx);

// Retrive a graph encoding context encoding phase.
EncodePhase GraphEncodeContext_GetEncodePhase(const GraphEncodeContext *ctx);

// Sets a graph encoding context phase.
void GraphEncodeContext_SetEncodePhase(GraphEncodeContext *ctx, EncodePhase phase);

// Retrive graph encoding context keys count.
uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx);

// Retrive graph encoding context proccessed key count.
uint64_t GraphEncodeContext_GetProccessedKeyCount(const GraphEncodeContext *ctx);

// Retrive graph encoding context proccessed nodes.
uint64_t GraphEncodeContext_GetProccessedNodes(const GraphEncodeContext *ctx);

// Set graph encoding context proccessed nodes.
void GraphEncodeContext_SetProcessedNodes(GraphEncodeContext *ctx, uint64_t nodes);

// Retrive graph encoding context proccessed deleted nodes.
uint64_t GraphEncodeContext_GetProccessedDeletedNodes(const GraphEncodeContext *ctx);

// Set graph encoding context proccessed deleted nodes.
void GraphEncodeContext_SetProcessedDeletedNodes(GraphEncodeContext *ctx, uint64_t deleted_nodes);

// Retrive graph encoding context proccessed edges.
uint64_t GraphEncodeContext_GetProccessedEdges(const GraphEncodeContext *ctx);

// Set graph encoding context proccessed edges.
void GraphEncodeContext_SetProcessedEdges(GraphEncodeContext *ctx, uint64_t edges);

// Retrive graph encoding context proccessed deleted edges.
uint64_t GraphEncodeContext_GetProccessedDeletedEdges(const GraphEncodeContext *ctx);

// Set graph encoding context proccessed deleted edges.
void GraphEncodeContext_SetProcessedDeletedEdges(GraphEncodeContext *ctx, uint64_t deleted_edges);

// Retrive graph encoding context stored datablock iterator.
DataBlockIterator *GraphEncodeContext_GetDatablockIterator(const GraphEncodeContext *ctx);

// Set graph encoding context datablock iterator.
void GraphEncodeContext_SetDatablockIterator(GraphEncodeContext *ctx, DataBlockIterator *iter);

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx);

// Increases the number of keys representing the graph, by a given delta.
void GraphEncodeContext_IncreaseKeyCount(GraphEncodeContext *ctx, uint64_t delta);

// Decrease the number of key representing the graph, by a given delta.
void GraphEncodeContext_DecreaseKeyCount(GraphEncodeContext *ctx, uint64_t delta);

// Increases the number of processed graph keys.
void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx);

// Free graph encoding context.
void GraphEncodeContext_Free(GraphEncodeContext *ctx);
