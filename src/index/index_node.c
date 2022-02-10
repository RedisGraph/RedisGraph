/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "index.h"
#include "../value.h"
#include "../query_ctx.h"
#include "../graph/graphcontext.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

extern RSDoc *Index_IndexGraphEntity(Index *idx,const GraphEntity *e,
		const void *key, size_t key_len, uint *doc_field_count);

void Index_IndexNode
(
	Index *idx,
	const Node *n
) {
	ASSERT(idx  !=  NULL);
	ASSERT(n    !=  NULL);

	RSIndex   *rsIdx           =  idx->idx;
	EntityID  key              =  ENTITY_GET_ID(n);
	size_t    key_len          =  sizeof(EntityID);
	uint      doc_field_count  =  0;

	RSDoc *doc = Index_IndexGraphEntity(
			idx, (const GraphEntity *)n, (const void *)&key, key_len,
			&doc_field_count);

	if(doc_field_count > 0) {
		RediSearch_SpecAddDocument(rsIdx, doc);
	} else {
		// entity doesn't poses any attributes which are indexed
		// remove entity from index and delete document
		Index_RemoveNode(idx, n);
		RediSearch_FreeDocument(doc);
	}
}

void populateNodeIndex
(
	Index *idx
) {
	ASSERT(idx != NULL);

	GraphContext  *gc  =  QueryCtx_GetGraphCtx();
	Graph         *g   =  gc->g;

	const RG_Matrix m = Graph_GetLabelMatrix(g, idx->label_id);
	ASSERT(m != NULL);

	RG_MatrixTupleIter it = {0};
	RG_MatrixTupleIter_reuse(&it, m);

	// iterate over each graph entity
	while(true) {
		EntityID id;
		bool depleted = false;

		RG_MatrixTupleIter_next_BOOL(&it, NULL, &id, NULL, &depleted);
		if(depleted) break;

		Node n;
		Graph_GetNode(g, id, &n);
		Index_IndexNode(idx, &n);
	}
}

void Index_RemoveNode
(
	Index *idx,    // index to update
	const Node *n  // node to remove from index
) {
	ASSERT(n   != NULL);
	ASSERT(idx != NULL);

	EntityID id = ENTITY_GET_ID(n);
	RediSearch_DeleteDocument(idx->idx, &id, sizeof(EntityID));
}

