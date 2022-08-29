/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "index.h"
#include "../query_ctx.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/edge.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

extern RSDoc *Index_IndexGraphEntity(Index *idx,const GraphEntity *e,
		const void *key, size_t key_len, uint *doc_field_count);

void Index_IndexEdge
(
	Index *idx,
	const Edge *e
) {
	ASSERT(idx  !=  NULL);
	ASSERT(e    !=  NULL);

	RSIndex   *rsIdx   =  idx->idx;
	EntityID  src_id   =  Edge_GetSrcNodeID(e);
	EntityID  dest_id  =  Edge_GetDestNodeID(e);
	EntityID  edge_id  =  ENTITY_GET_ID(e);

	EdgeIndexKey key = {.src_id = src_id, .dest_id = dest_id, .edge_id = edge_id};
	size_t key_len = sizeof(EdgeIndexKey);

	uint doc_field_count = 0;
	RSDoc *doc = Index_IndexGraphEntity(
			idx, (const GraphEntity *)e, (const void *)&key, key_len,
			&doc_field_count);

	if(doc_field_count > 0) {
		// add src_node and dest_node fields
		RediSearch_DocumentAddFieldNumber(doc, "_src_id", src_id,
				RSFLDTYPE_NUMERIC);

		RediSearch_DocumentAddFieldNumber(doc, "_dest_id", dest_id,
				RSFLDTYPE_NUMERIC);

		RediSearch_SpecAddDocument(rsIdx, doc);
	} else {
		// entity doesn't possess any attributes which are indexed
		// remove entity from index and delete document
		Index_RemoveEdge(idx, e);
		RediSearch_FreeDocument(doc);
	}
}

void populateEdgeIndex
(
	Index *idx
) {
	ASSERT(idx != NULL);

	GraphContext  *gc  =  QueryCtx_GetGraphCtx();
	Graph         *g   =  gc->g;

	const RG_Matrix m = Graph_GetRelationMatrix(g, idx->label_id, false);
	ASSERT(m != NULL);

	RG_MatrixTupleIter it;
	RG_MatrixTupleIter_reuse(&it, m);

	// iterate over each graph entity
	while(true) {
		bool      depleted;
		EntityID  src_id;
		EntityID  dest_id;
		EntityID  edge_id;

		RG_MatrixTupleIter_next(&it, &src_id, &dest_id, &edge_id, &depleted);
		if(depleted) break;

		Edge e;
		e.relationID  =  idx->label_id;
		e.srcNodeID   =  src_id;
		e.destNodeID  =  dest_id;

		if(SINGLE_EDGE(edge_id)) {
			Graph_GetEdge(g, edge_id, &e);
			Index_IndexEdge(idx, &e);
		} else {
			EdgeID *edgeIds = (EdgeID *)(CLEAR_MSB(edge_id));
			uint edgeCount = array_len(edgeIds);

			for(uint i = 0; i < edgeCount; i++) {
				edge_id       = edgeIds[i];
				Graph_GetEdge(g, edge_id, &e);
				Index_IndexEdge(idx, &e);
			}
		}
	}
}

void Index_RemoveEdge
(
	Index *idx,    // index to update
	const Edge *e  // edge to remove from index
) {
	ASSERT(e   != NULL);
	ASSERT(idx != NULL);

	EntityID  src_id   =  Edge_GetSrcNodeID(e);
	EntityID  dest_id  =  Edge_GetDestNodeID(e);
	EntityID  edge_id  =  ENTITY_GET_ID(e);

	EdgeIndexKey key = {.src_id = src_id, .dest_id = dest_id, .edge_id = edge_id};
	size_t key_len = sizeof(EdgeIndexKey);
	RediSearch_DeleteDocument(idx->idx, &key, key_len);
}

