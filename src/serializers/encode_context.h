/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "../util/datablock/datablock.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "rax.h"

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
	uint64_t keys_processed;                    // Count the number of procssed graph keys.
	rax *keys;                                  // The name of the meta keys representing the graph.
	EncodePhase phase;                          // Represents the current encoding phase.
	uint64_t processed_nodes;                   // Number of encoded nodes.
	uint64_t processed_deleted_nodes;           // Number of encoded deleted nodes.
	uint64_t processed_edges;                   // Number of encoded edges.
	uint64_t processed_deleted_edges;           // Number of encoded deleted edges.
	DataBlockIterator *datablock_iterator;      // Datablock iterator to be saved in the context.
	uint current_relation_matrix_id;            // Current encoded relationship matrix.
	GxB_MatrixTupleIter *matrix_tuple_iterator; // Matrix tuple iterator to be saved in the context.
} GraphEncodeContext;

// Creates a new graph encoding context.
GraphEncodeContext *GraphEncodeContext_New();

// Reset a graph encoding context.
void GraphEncodeContext_Reset(GraphEncodeContext *ctx);

// Retrieve the graph current encoding phase.
EncodePhase GraphEncodeContext_GetEncodePhase(const GraphEncodeContext *ctx);

// Sets the graph current encoding phase.
void GraphEncodeContext_SetEncodePhase(GraphEncodeContext *ctx, EncodePhase phase);

// Retrieve the graph representing keys count.
uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx);

// Retrieve graph currently processed key count - keys processed so far.
uint64_t GraphEncodeContext_GetProcessedKeyCount(const GraphEncodeContext *ctx);

// Retrieve graph currently encoded nodes count - nodes encoded so far.
uint64_t GraphEncodeContext_GetProcessedNodesCount(const GraphEncodeContext *ctx);

// Update the graph currently encoded nodes count.
void GraphEncodeContext_SetProcessedNodesCount(GraphEncodeContext *ctx, uint64_t nodes);

// Retrieve graph currently encoded deleted nodes count - deleted nodes encoded so far.
uint64_t GraphEncodeContext_GetProcessedDeletedNodesCount(const GraphEncodeContext *ctx);

// Update the graph currently encoded deleted nodes count.
void GraphEncodeContext_SetProcessedDeletedNodesCount(GraphEncodeContext *ctx,
													  uint64_t deleted_nodes);

// Retrieve graph currently encoded edges count - edges encoded so far.
uint64_t GraphEncodeContext_GetProcessedEdgesCount(const GraphEncodeContext *ctx);

// Update the graph currently encoded edges count.
void GraphEncodeContext_SetProcessedEdgesCount(GraphEncodeContext *ctx, uint64_t edges);

// Retrieve graph currently encoded deleted edges count - deleted edges encoded so far.
uint64_t GraphEncodeContext_GetProcessedDeletedEdgesCount(const GraphEncodeContext *ctx);

// Update the graph currently encoded deleted edges count.
void GraphEncodeContext_SetProcessedDeletedEdgesCount(GraphEncodeContext *ctx,
													  uint64_t deleted_edges);

// Retrieve stored datablock iterator.
DataBlockIterator *GraphEncodeContext_GetDatablockIterator(const GraphEncodeContext *ctx);

// Set graph encoding context datablock iterator - keep iterator state for further usage.
void GraphEncodeContext_SetDatablockIterator(GraphEncodeContext *ctx, DataBlockIterator *iter);

// Retrieve graph encoding context current encoded relation matrix id.
uint GraphEncodeContex_GetCurrentRelationID(const GraphEncodeContext *ctx);

// Set graph encoding context current encoded relation matrix id.
void GraphEncodeContex_SetCurrentRelationID(GraphEncodeContext *ctx,
											uint current_relation_matrix_id);

// Retrieve stored matrix tuple iterator.
GxB_MatrixTupleIter *GraphEncodeContext_GetMatrixTupleIterator(const GraphEncodeContext *ctx);

// Set graph encoding context matrix tuple iterator - keep iterator state for further usage.
void GraphEncodeContext_SetMatrixTupleIterator(GraphEncodeContext *ctx, GxB_MatrixTupleIter *iter);

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx);

// Save a new key representing the graph.
void GraphEncodeContext_AddKey(GraphEncodeContext *ctx, const char *key);

// Remove a key represeting the graph.
void GraphEncodeContext_DeleteKey(GraphEncodeContext *ctx, const char *key);

// Increases the number of processed graph keys.
void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx);

// Return the graph virtual keys.
unsigned char **GraphEncodeContext_GetKeys(GraphEncodeContext *ctx);

// Free graph encoding context.
void GraphEncodeContext_Free(GraphEncodeContext *ctx);
