/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_context.h"
#include "../RG.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"

GraphEncodeContext *GraphEncodeContext_New() {
	GraphEncodeContext *ctx = rm_calloc(1, sizeof(GraphEncodeContext));
	ctx->meta_keys = raxNew();
	GraphEncodeContext_Reset(ctx);
	return ctx;
}

static void _GraphEncodeContext_ResetHeader(GraphEncodeContext *ctx) {
	ASSERT(ctx != NULL);

	GraphEncodeHeader *header = &(ctx->header);

	header->key_count = 0;
	header->node_count = 0;
	header->edge_count = 0;
	header->graph_name = NULL;
	header->label_matrix_count = 0;
	header->relationship_matrix_count = 0;

	if(header->multi_edge != NULL) {
		rm_free(header->multi_edge);
		header->multi_edge = NULL;
	}
}

void GraphEncodeContext_Reset(GraphEncodeContext *ctx) {
	ASSERT(ctx != NULL);

	_GraphEncodeContext_ResetHeader(ctx);

	ctx->offset = 0;
	ctx->keys_processed = 0;
	ctx->state = ENCODE_STATE_INIT;
	ctx->multiple_edges_src_id = 0;
	ctx->multiple_edges_dest_id = 0;
	ctx->multiple_edges_array = NULL;
	ctx->current_relation_matrix_id = 0;
	ctx->multiple_edges_current_index = 0;

	// Avoid leaks in case or reset during encodeing.
	if(ctx->datablock_iterator != NULL) {
		DataBlockIterator_Free(ctx->datablock_iterator);
		ctx->datablock_iterator = NULL;
	}

	// Avoid leaks in case or reset during encodeing.
	if(ctx->matrix_tuple_iterator != NULL) {
		GxB_MatrixTupleIter_free(ctx->matrix_tuple_iterator);
		ctx->matrix_tuple_iterator = NULL;
	}
}

// Determine if matrix 'R' contains entries representing multiple edges
// TODO: consider relocating this function
static bool _MultiEdgeMatrix(GrB_Matrix R, GrB_Monoid min_monoid) {
	GrB_Info info;
	UNUSED(info);
	bool multi_edge;
	uint64_t edgeID = 0;

	// To determine if matrix R contains an entry which represents
	// multiple edges, we need to find the minimum value entry
	// as a single-edge entry has its MSB turned on, by searching for the
	// minimum value entry we'll get an entry with a pointer value in
	// it (if such exists) as these have thier MSB turned off
	info = GrB_Matrix_reduce_UINT64(&edgeID, NULL, min_monoid, R, NULL);
	ASSERT(info == GrB_SUCCESS);
	multi_edge = !(SINGLE_EDGE(edgeID));

	return multi_edge;
}

void GraphEncodeContext_InitHeader(GraphEncodeContext *ctx, const char *graph_name, Graph *g) {
	ASSERT(g != NULL);
	ASSERT(ctx != NULL);

	int r_count = Graph_RelationTypeCount(g);
	GraphEncodeHeader *header = &(ctx->header);
	ASSERT(header->multi_edge == NULL);

	header->graph_name = graph_name;
	header->node_count = Graph_NodeCount(g);
	header->edge_count = Graph_EdgeCount(g);
	header->relationship_matrix_count = r_count;
	header->label_matrix_count = Graph_LabelTypeCount(g);
	header->key_count = GraphEncodeContext_GetKeyCount(ctx);
	header->multi_edge = rm_malloc(sizeof(bool) * r_count);

	// denote for each relationship matrix Ri if it contains muti-edge entries
	// this information alows for an optimization when loading the data
	// as construction of a matrix without multiple edge entry is cheaper
	for(uint i = 0; i < r_count; i++) {
		bool multi_edge = Graph_RelationshipContainsMultiEdge(g, i, false);
		header->multi_edge[i] = multi_edge;
	}
}

EncodeState GraphEncodeContext_GetEncodeState(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->state;
}

void GraphEncodeContext_SetEncodeState(GraphEncodeContext *ctx, EncodeState state) {
	ASSERT(ctx);
	ctx->state = state;
}

uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	// The `meta_keys` rax contains only the meta keys names. Add one for the graph context key.
	return raxSize(ctx->meta_keys) + 1;
}

void GraphEncodeContext_AddMetaKey(GraphEncodeContext *ctx, const char *key) {
	ASSERT(ctx);
	raxInsert(ctx->meta_keys, (unsigned char *)key, strlen(key), NULL, NULL);
}

unsigned char **GraphEncodeContext_GetMetaKeys(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return raxKeys(ctx->meta_keys);
}

void GraphEncodeContext_ClearMetaKeys(GraphEncodeContext *ctx) {
	ASSERT(ctx);
	raxFree(ctx->meta_keys);
	ctx->meta_keys = raxNew();
}

uint64_t GraphEncodeContext_GetProcessedKeyCount(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->keys_processed;
}

uint64_t GraphEncodeContext_GetProcessedEntitiesOffset(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->offset;
}

void GraphEncodeContext_SetProcessedEntitiesOffset(GraphEncodeContext *ctx,
												   uint64_t offset) {
	ASSERT(ctx);
	ctx->offset = offset;
}

DataBlockIterator *GraphEncodeContext_GetDatablockIterator(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->datablock_iterator;
}

void GraphEncodeContext_SetDatablockIterator(GraphEncodeContext *ctx,
											 DataBlockIterator *iter) {
	ASSERT(ctx);
	ctx->datablock_iterator = iter;
}

uint GraphEncodeContext_GetCurrentRelationID(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->current_relation_matrix_id;
}

void GraphEncodeContext_SetCurrentRelationID(GraphEncodeContext *ctx,
											 uint current_relation_matrix_id) {
	ASSERT(ctx);
	ctx->current_relation_matrix_id = current_relation_matrix_id;
}

GxB_MatrixTupleIter *GraphEncodeContext_GetMatrixTupleIterator(
	const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->matrix_tuple_iterator;
}

void GraphEncodeContext_SetMatrixTupleIterator(GraphEncodeContext *ctx,
											   GxB_MatrixTupleIter *iter) {
	ASSERT(ctx);
	ctx->matrix_tuple_iterator = iter;
}

void GraphEncodeContext_SetMutipleEdgesArray(GraphEncodeContext *ctx, EdgeID *edges,
											 uint current_index, NodeID src, NodeID dest) {
	ASSERT(ctx);
	ctx->multiple_edges_array = edges;
	ctx->multiple_edges_current_index = current_index;
	ctx->multiple_edges_src_id = src;
	ctx->multiple_edges_dest_id = dest;
}

EdgeID *GraphEncodeContext_GetMultipleEdgesArray(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->multiple_edges_array;
}

uint GraphEncodeContext_GetMultipleEdgesCurrentIndex(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->multiple_edges_current_index;
}


NodeID GraphEncodeContext_GetMultipleEdgesSourceNode(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->multiple_edges_src_id;
}

NodeID GraphEncodeContext_GetMultipleEdgesDestinationNode(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->multiple_edges_dest_id;
}

bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx) {
	ASSERT(ctx);
	return ctx->keys_processed == GraphEncodeContext_GetKeyCount(ctx);
}

void GraphEncodeContext_IncreaseProcessedKeyCount(GraphEncodeContext *ctx) {
	ASSERT(ctx);
	ASSERT(ctx->keys_processed < GraphEncodeContext_GetKeyCount(ctx));
	ctx->keys_processed++;
}

static void GraphEncodeContext_FreeHeader(GraphEncodeContext *ctx) {
	if(ctx->header.multi_edge != NULL) rm_free(ctx->header.multi_edge);
}

void GraphEncodeContext_Free(GraphEncodeContext *ctx) {
	if(ctx) {
		GraphEncodeContext_FreeHeader(ctx);
		raxFree(ctx->meta_keys);
		rm_free(ctx);
	}
}

