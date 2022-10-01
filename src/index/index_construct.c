/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "index.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

#include <assert.h>

// called by Index_Construct
// responsible for creating the index structure only!
// e.g. fields, stopwords, language
RSIndex *_Index_ConstructStructure
(
	Index *idx
) {
	// TODO: at which point do we need to acquire Redis's GIL?
	RSIndex *rsIdx = NULL;
	RSIndexOptions *idx_options = RediSearch_CreateIndexOptions();
	RediSearch_IndexOptionsSetLanguage(idx_options, idx->language);
	// TODO: Remove this comment when https://github.com/RediSearch/RediSearch/issues/1100 is closed
	// RediSearch_IndexOptionsSetGetValueCallback(idx_options, _getNodeAttribute, gc);

	// enable GC, every 30 seconds gc will check if there's garbage
	// if there are over 100 docs to remove GC will perform clean up
	RediSearch_IndexOptionsSetGCPolicy(idx_options, GC_POLICY_FORK);

	if(idx->stopwords) {
		RediSearch_IndexOptionsSetStopwords(idx_options,
				(const char**)idx->stopwords, array_len(idx->stopwords));
	} else if(idx->type == IDX_EXACT_MATCH) {
		RediSearch_IndexOptionsSetStopwords(idx_options, NULL, 0);
	}

	rsIdx = RediSearch_CreateIndex(idx->label, idx_options);
	RediSearch_FreeIndexOptions(idx_options);

	// create indexed fields
	uint fields_count = array_len(idx->fields);
	if(idx->type == IDX_FULLTEXT) {
		for(uint i = 0; i < fields_count; i++) {
			IndexField *field = idx->fields + i;
			// introduce text field
			unsigned options = RSFLDOPT_NONE;
			if(field->nostem) options |= RSFLDOPT_TXTNOSTEM;

			if(strcmp(field->phonetic, INDEX_FIELD_DEFAULT_PHONETIC) != 0) {
				options |= RSFLDOPT_TXTPHONETIC;
			}

			RSFieldID fieldID = RediSearch_CreateField(rsIdx, field->name,
					RSFLDTYPE_FULLTEXT, options);
			RediSearch_TextFieldSetWeight(rsIdx, fieldID, field->weight);
		}
	} else {
		for(uint i = 0; i < fields_count; i++) {
			IndexField *field = idx->fields + i;
			// introduce both text, numeric and geo fields
			unsigned types = RSFLDTYPE_NUMERIC | RSFLDTYPE_GEO | RSFLDTYPE_TAG;
			RSFieldID fieldID = RediSearch_CreateField(rsIdx, field->name,
					types, RSFLDOPT_NONE);

			RediSearch_TagFieldSetSeparator(rsIdx, fieldID, INDEX_SEPARATOR);
			RediSearch_TagFieldSetCaseSensitive(rsIdx, fieldID, 1);
		}

		// for none indexable types e.g. Array introduce an additional field
		// "none_indexable_fields" which will hold a list of attribute names
		// that were not indexed
		RSFieldID fieldID = RediSearch_CreateField(rsIdx,
				INDEX_FIELD_NONE_INDEXED, RSFLDTYPE_TAG, RSFLDOPT_NONE);

		RediSearch_TagFieldSetSeparator(rsIdx, fieldID, INDEX_SEPARATOR);
		RediSearch_TagFieldSetCaseSensitive(rsIdx, fieldID, 1);
	}

	return rsIdx;
}

// index nodes in an asynchronous manner
// nodes are being indexed in batchs while the graph's read lock is held
// to avoid interfering with the DB ongoing operation after each batch of nodes
// is indexed the graph read lock is released
// alowing for write queries to be processed
//
// it is safe to run a write query which effects the index by either:
// adding/removing/updating an entity while the index is being populated
// in the "worst" case we will index that entity twice which is perfectly OK
static void _Index_PopulateNodeIndex
(
	Index *idx,
	Graph *g
) {
	ASSERT(g   != NULL);
	ASSERT(idx != NULL);

	GrB_Index          rowIdx     = 0;
	int                indexed    = 0;     // number of entities indexed in current batch
	int                batch_size = 1000;  // max number of entities to index in one go
	RG_MatrixTupleIter it         = {0};

	while(true) {
		// index state changed, abort indexing
		// this can happen if for example the following sequance is issued:
		// 1. CREATE INDEX FOR (n:Person) ON (n.age)
		// 2. CREATE INDEX FOR (n:Person) ON (n.height)
		if(idx->state != IDX_POPULATING) {
			break;
		}

		// reset number of indexed nodes in batch
		indexed = 0;

		// lock graph for reading
		Graph_AcquireReadLock(g);

		// fetch label matrix
		const RG_Matrix m = Graph_GetLabelMatrix(g, idx->label_id);
		ASSERT(m != NULL);

		//----------------------------------------------------------------------
		// resume scanning from rowIdx
		//----------------------------------------------------------------------

		RG_MatrixTupleIter_attach(&it, m);
		if(RG_MatrixTupleIter_jump_to_row(&it, rowIdx) != GrB_SUCCESS) {
			break;
		}

		//----------------------------------------------------------------------
		// batch index nodes
		//----------------------------------------------------------------------

		EntityID id;
		while(indexed < batch_size &&
			  RG_MatrixTupleIter_next_BOOL(&it, &id, NULL, NULL) == GrB_SUCCESS)
		{
			Node n;
			Graph_GetNode(g, id, &n);
			Index_IndexNode(idx, &n);
			indexed++;
		}

		// release read lock
		Graph_ReleaseLock(g);

		if(indexed != batch_size) {
			// iterator depleted, no more nodes to index
			break;
		} else {
			// finished current batch
			RG_MatrixTupleIter_detach(&it);

			// continue next batch from row id+1
			// this is true because we're iterating over a diagonal matrix
			rowIdx = id + 1;
		}
	}

	RG_MatrixTupleIter_detach(&it);
}

// index edges in an asynchronous manner
// edges are being indexed in batchs while the graph's read lock is held
// to avoid interfering with the DB ongoing operation after each batch of edges
// is indexed the graph read lock is released
// alowing for write queries to be processed
//
// it is safe to run a write query which effects the index by either:
// adding/removing/updating an entity while the index is being populated
// in the "worst" case we will index that entity twice which is perfectly OK
static void _Index_PopulateEdgeIndex
(
	Index *idx,
	Graph *g
) {
	ASSERT(g   != NULL);
	ASSERT(idx != NULL);


	GrB_Info  info;
	EntityID  src_id       = 0;     // current processed row idx
	EntityID  dest_id      = 0;     // current processed column idx
	EntityID  edge_id      = 0;     // current processed edge id
	EntityID  prev_src_id  = 0;     // last processed row idx
	EntityID  prev_dest_id = 0;     // last processed column idx
	int       indexed      = 0;     // number of entities indexed in current batch
	int       batch_size   = 1000;  // max number of entities to index in one go
	RG_MatrixTupleIter it  = {0};

	while(true) {
		// index state changed, abort indexing
		// this can happen if for example the following sequance is issued:
		// 1. CREATE INDEX FOR (:Person)-[e:WORKS]-(:Company) ON (e.since)
		// 2. CREATE INDEX FOR (:Person)-[e:WORKS]-(:Company) ON (e.title)
		if(idx->state != IDX_POPULATING) {
			break;
		}

		// reset number of indexed edges in batch
		indexed      = 0;
		prev_src_id  = src_id;
		prev_dest_id = dest_id;

		// lock graph for reading
		Graph_AcquireReadLock(g);

		// fetch relation matrix
		const RG_Matrix m = Graph_GetRelationMatrix(g, idx->label_id, false);
		ASSERT(m != NULL);

		//----------------------------------------------------------------------
		// resume scanning from previous row/col indices
		//----------------------------------------------------------------------

		RG_MatrixTupleIter_attach(&it, m);
		if(RG_MatrixTupleIter_jump_to_row(&it, src_id) != GrB_SUCCESS) {
			break;
		}

		// skip previously indexed edges
		while((info = RG_MatrixTupleIter_next_UINT64(&it, &src_id, &dest_id,
						NULL)) == GrB_SUCCESS &&
				src_id == prev_src_id &&
				dest_id <= prev_dest_id);

		// process only if iterator is on an active entry
		if(info != GrB_SUCCESS) {
			break;
		}

		//----------------------------------------------------------------------
		// batch index edges
		//----------------------------------------------------------------------

		do {
			Edge e;
			e.srcNodeID  = src_id;
			e.destNodeID = dest_id;
			e.relationID = idx->label_id;

			if(SINGLE_EDGE(edge_id)) {
				Graph_GetEdge(g, edge_id, &e);
				Index_IndexEdge(idx, &e);
			} else {
				EdgeID *edgeIds = (EdgeID *)(CLEAR_MSB(edge_id));
				uint edgeCount = array_len(edgeIds);

				for(uint i = 0; i < edgeCount; i++) {
					edge_id = edgeIds[i];
					Graph_GetEdge(g, edge_id, &e);
					Index_IndexEdge(idx, &e);
				}
			}
			indexed++;
		} while(indexed < batch_size &&
			  RG_MatrixTupleIter_next_UINT64(&it, &src_id, &dest_id, &edge_id)
				== GrB_SUCCESS);

		// release read lock
		Graph_ReleaseLock(g);

		if(indexed != batch_size) {
			// iterator depleted, no more edges to index
			break;
		} else {
			// finished current batch
			RG_MatrixTupleIter_detach(&it);
		}
	}

	RG_MatrixTupleIter_detach(&it);
}

// constructs index
void Index_Populate
(
	Index *idx,
	Graph *g
) {
	ASSERT(g   != NULL);
	ASSERT(idx != NULL);

	ASSERT(idx->state == IDX_POPULATING);

	// check if RediSearch index already exists
	// in which case we'll simply drop the index and recreate it
	if(idx->idx != NULL) {
		// TODO: do we need to acquire GIL?
		printf("TODO: do we need to acquire GIL?\n");

		RediSearch_DropIndex(idx->idx);
		idx->idx = NULL;
	}

	//--------------------------------------------------------------------------
	// create RediSearch index structure
	//--------------------------------------------------------------------------

	idx->idx = _Index_ConstructStructure(idx);

	//--------------------------------------------------------------------------
	// populate index
	//--------------------------------------------------------------------------

	if(idx->entity_type == GETYPE_NODE) {
		_Index_PopulateNodeIndex(idx, g);
	} else {
		_Index_PopulateEdgeIndex(idx, g);
	}

	// try to enable index
	Index_Enable(idx);
}

