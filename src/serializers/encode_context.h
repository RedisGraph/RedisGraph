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
#include "../graph/entities/graph_entity.h"
#include "rax.h"

// Represent a graph encoding state.
typedef enum {
	ENCODE_STATE_INIT,
	ENCODE_STATE_NODES,
	ENCODE_STATE_DELETED_NODES,
	ENCODE_STATE_EDGES,
	ENCODE_STATE_DELETED_EDGES,
	ENCODE_STATE_GRAPH_SCHEMA,
	ENCODE_STATE_FINAL
} EncodeState;

// A struct that maintains the state of a graph encoding to RDB or encode from RDB.
typedef struct {
	uint64_t keys_processed;                    // Count the number of procssed graph keys.
	rax *meta_keys;                             // The holds the names of meta keys representing the graph.
	EncodeState state;                          // Represents the current encoding state.
	uint64_t offset;                            // Number of encoded entities in the current state.
	DataBlockIterator *datablock_iterator;      // Datablock iterator to be saved in the context.
	uint current_relation_matrix_id;            // Current encoded relationship matrix.
	GxB_MatrixTupleIter *matrix_tuple_iterator; // Matrix tuple iterator to be saved in the context.
	EdgeID *multiple_edges_array;               // Multiple edges array, save in the context.
	uint multiple_edges_current_index;          // The current index of the encoded edges array.
	NodeID multiple_edges_src_id;               // The current edges array sourc node id.
	NodeID multiple_edges_dest_id;              // The current edges array destination node id.
} GraphEncodeContext;

// Creates a new graph encoding context.
GraphEncodeContext *GraphEncodeContext_New();

// Reset a graph encoding context.
void GraphEncodeContext_Reset(GraphEncodeContext *ctx);

// Retrieve the graph current encoding phase.
EncodeState GraphEncodeContext_GetEncodeState(const GraphEncodeContext *ctx);

// Sets the graph current encoding phase.
void GraphEncodeContext_SetEncodeState(GraphEncodeContext *ctx, EncodeState phase);

// Retrieve the graph representing keys count.
uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx);

// Add a meta key name, required for encoding the graph.
void GraphEncodeContext_AddMetaKey(GraphEncodeContext *ctx, const char *key);

// Returns a dynamic array with copies of the meta key names.
unsigned char **GraphEncodeContext_GetMetaKeys(const GraphEncodeContext *ctx);

// Removes the stored meta key names from the context.
void GraphEncodeContext_ClearMetaKeys(GraphEncodeContext *ctx);

// Retrieve graph currently processed key count - keys processed so far.
uint64_t GraphEncodeContext_GetProcessedKeyCount(const GraphEncodeContext *ctx);

// Retrieve graph entities encoded so far in the current state.
uint64_t GraphEncodeContext_GetProcessedEntitiesOffset(const GraphEncodeContext *ctx);

// Update the graph entities encoded so far in the current state.
void GraphEncodeContext_SetProcessedEntitiesOffset(GraphEncodeContext *ctx, uint64_t offset);

// Retrieve stored datablock iterator.
DataBlockIterator *GraphEncodeContext_GetDatablockIterator(const GraphEncodeContext *ctx);

// Set graph encoding context datablock iterator - keep iterator state for further usage.
void GraphEncodeContext_SetDatablockIterator(GraphEncodeContext *ctx, DataBlockIterator *iter);

// Retrieve graph encoding context current encoded relation matrix id.
uint GraphEncodeContext_GetCurrentRelationID(const GraphEncodeContext *ctx);

// Set graph encoding context current encoded relation matrix id.
void GraphEncodeContext_SetCurrentRelationID(GraphEncodeContext *ctx,
											 uint current_relation_matrix_id);

// Retrieve stored matrix tuple iterator.
GxB_MatrixTupleIter *GraphEncodeContext_GetMatrixTupleIterator(const GraphEncodeContext *ctx);

// Set graph encoding context matrix tuple iterator - keep iterator state for further usage.
void GraphEncodeContext_SetMatrixTupleIterator(GraphEncodeContext *ctx, GxB_MatrixTupleIter *iter);

// Sets a multiple edges array and the current index, for saving the state of multiple edges encoding.
void GraphEncodeContext_SetMutipleEdgesArray(GraphEncodeContext *ctx, EdgeID *edges,
											 uint current_index, NodeID src, NodeID dest);

// Retrive the multiple edges array, to continue array of multiple edge encoding.
EdgeID *GraphEncodeContext_GetMultipleEdgesArray(const GraphEncodeContext *ctx);

// Retrive the multiple edges array current index, to continue array of multiple edge encoding.
uint GraphEncodeContext_GetMultipleEdgesCurrentIndex(const GraphEncodeContext *ctx);

// Retrive the multiple edges array source node.
NodeID GraphEncodeContext_GetMultipleEdgesSourceNode(const GraphEncodeContext *ctx);

// Retrive the multiple edges array destination node.
NodeID GraphEncodeContext_GetMultipleEdgesDestinationNode(const GraphEncodeContext *ctx);

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx);

// Increases the number of processed graph keys.
void GraphEncodeContext_IncreaseProcessedKeyCount(GraphEncodeContext *ctx);

// Free graph encoding context.
void GraphEncodeContext_Free(GraphEncodeContext *ctx);
