/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "index.h"
#include "../value.h"
#include "../query_ctx.h"
#include "../graph/graphcontext.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

extern RSDoc *Index_IndexGraphEntity(Index idx, const GraphEntity *e,
		const void *key, size_t key_len, uint *doc_field_count);

void Index_IndexNode
(
	Index idx,
	const Node *n
) {
	ASSERT(idx  !=  NULL);
	ASSERT(n    !=  NULL);

	EntityID key             = ENTITY_GET_ID(n);
	RSDoc    *doc            = NULL;
	size_t   key_len         = sizeof(EntityID);
	RSIndex  *active_Idx     = Index_ActiveRSIndex(idx);
	RSIndex  *pending_Idx    = Index_PendingRSIndex(idx);
	uint     doc_field_count = 0;

	// create RediSearch document representing node
	doc = Index_IndexGraphEntity(idx, (const GraphEntity *)n,
			(const void *)&key, key_len, &doc_field_count);

	if(doc_field_count == 0) {
		// entity doesn't poses any attributes which are indexed
		// remove entity from index and delete document
		Index_RemoveNode(idx, n);
		RediSearch_FreeDocument(doc);
		return;
	}

	// add document to active RediSearch index if present
	if(active_Idx != NULL) {
		RediSearch_SpecAddDocument(active_Idx, doc);
		doc = NULL;
	}

	// add document to pending RediSearch index if present
	if(pending_Idx != NULL) {
		// recreate document if used by active index
		if(doc == NULL) {
			// TODO: RediSearch API clone DOC
			doc = Index_IndexGraphEntity(idx, (const GraphEntity *)n,
					(const void *)&key, key_len, &doc_field_count);
		}
		RediSearch_SpecAddDocument(pending_Idx, doc);
	}
}

void Index_RemoveNode
(
	Index idx,     // index to update
	const Node *n  // node to remove from index
) {
	ASSERT(n   != NULL);
	ASSERT(idx != NULL);

	EntityID id           = ENTITY_GET_ID(n);
	RSIndex  *active_Idx  = Index_ActiveRSIndex(idx);
	RSIndex  *pending_Idx = Index_PendingRSIndex(idx);

	if(active_Idx != NULL) {
		RediSearch_DeleteDocument(active_Idx, &id, sizeof(EntityID));
	}
	if(pending_Idx != NULL) {
		RediSearch_DeleteDocument(pending_Idx, &id, sizeof(EntityID));
	}
}

