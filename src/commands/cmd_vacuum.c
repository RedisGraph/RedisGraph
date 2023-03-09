/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "../graph/graphcontext.h"
#include "../util/datablock/datablock.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

#include <stdint.h>

static int _cmp
(
	const void *a,
	const void *b
) {
	return (*(uint64_t*)b - *(uint64_t*)a);
}

// vacuum migrates entities within our datablock into deleted positions
//
// the following datablock contains:
// 5 elements: '*'
// 3 deleted slots: 'D'
// 3 empty unused slots: '_'
//
// [*,D,*,D,D,*,*,*,_,_,_]
//
// the migration process moves elements from the end of the datablock to occupy
// deleted slots, after the process is done the datablock would look like:
//
// [*,*,*,*,*,_,_,_,_,_,_]
//
// to free up space empty blocks are freed
static uint64_t _Vacuum
(
	Graph *g
) {
	GrB_Index  n;     // dimention of matrices
	GrB_Vector v;     // temporary vector
	GrB_Matrix PR;    // rows permutation matrix
	GrB_Matrix PC;    // columns permutation matrix
	GrB_Info   info;  // GraphBLAS return code

	// determine number of deleted nodes
	bool       x             = true;
	DataBlock *nodes         = g->nodes;
	uint64_t  *free_list     = nodes->deletedIdx;
	uint32_t   free_list_len = DataBlock_DeletedItemsCount(nodes);

	if(free_list_len == 0) {
		// no fragmantation
		return 0;
	}

	// determin if there's a chance of freeing blocks
	uint64_t cap       = DataBlock_ItemCap(nodes);
	uint64_t count     = DataBlock_ItemCount(nodes);
	uint64_t block_cap = DataBlock_BlockCap(nodes);

	// we will be able to free space only if number of available space
	// in the datablock is greater than a block capacity
	uint64_t available = (cap - count) + free_list_len;
	if(available < block_cap) {
		// not enough empty slots
		return 0;
	}

	// TODO: i believe in this case PC = PR transposed
	// TODO: find a better way of constructing I

	//--------------------------------------------------------------------------
	// build a the identity NxN matrix
	//--------------------------------------------------------------------------

	n = Graph_RequiredMatrixDim(g);

	info = GrB_Vector_new(&v, GrB_BOOL, n);
	ASSERT(info == GrB_SUCCESS);

	// populate entire vector with 'true' values
	info = GrB_Vector_assign_BOOL(v, NULL, NULL, true, GrB_ALL, n, NULL);
	ASSERT(info == GrB_SUCCESS);

	// assign vector to matrix main diagonal
	// PR = I
	info = GrB_Matrix_diag(&PR, v, 0);
	ASSERT(info == GrB_SUCCESS);

	// free vector
	info = GrB_free(&v);
	ASSERT(info == GrB_SUCCESS);

	// sort deleted node list descending
	qsort(free_list, free_list_len, sizeof(uint64_t), _cmp);

	// scan nodes from end to start
	DataBlockIterator *it = DataBlock_ScanDesc(nodes);

	//--------------------------------------------------------------------------
	// relocate nodes
	//--------------------------------------------------------------------------

	uint64_t src;   // migrated node original ID
	uint64_t dest;  // migrated node new ID

	// as long as there are nodes to migrate
	// and node migration succedded
	while(DataBlockIterator_Next(it, &src) != NULL &&
		  DataBlock_MigrateItem(nodes, src, &dest) == true) {

		// node with ID 'src' has been migrated to ID 'dest'
		ASSERT(src > dest);

		// migrate row 'src' to row 'dest'
		info = GrB_Matrix_setElement_BOOL(PR, true, dest, src);
		ASSERT(info == GrB_SUCCESS);

		// clear out migrated row
		GrB_Matrix_removeElement(PR, src, src);
		ASSERT(info == GrB_SUCCESS);

		// clear out destination row
		GrB_Matrix_removeElement(PR, dest, dest);
		ASSERT(info == GrB_SUCCESS);
	}

	GxB_print(PR, GxB_SHORT);

	//--------------------------------------------------------------------------
	// adjust graph matrices
	//--------------------------------------------------------------------------

	Graph_ApplyAllPending(g, true);

	//--------------------------------------------------------------------------
	// swap rows PR * M
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// adjust label matrices
	//--------------------------------------------------------------------------

	int labels_count = Graph_LabelTypeCount(g);
	for(int i = 0; i < labels_count; i++) {
		GrB_Matrix M = RG_MATRIX_M(Graph_GetLabelMatrix(g, i));
		info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, PR, M, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	int reltype_count = Graph_RelationTypeCount(g);
	for(int i = 0; i < reltype_count; i++) {
		GrB_Matrix M = RG_MATRIX_M(Graph_GetRelationMatrix(g, i, false));
		info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_UINT64, PR, M, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	//--------------------------------------------------------------------------
	// adjust adjacency matrx
	//--------------------------------------------------------------------------

	GrB_Matrix M = RG_MATRIX_M(Graph_GetAdjacencyMatrix(g, false));
	info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, PR, M, NULL);
	ASSERT(info == GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// adjust node labels matrix
	//--------------------------------------------------------------------------

	M = RG_MATRIX_M(Graph_GetNodeLabelMatrix(g));
	info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, PR, M, NULL);
	ASSERT(info == GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// swap columns  M * PC
	//--------------------------------------------------------------------------

	// PC = PR transposed
	PC = PR;
	info = GrB_transpose(PC, NULL, NULL, PR, NULL);
	ASSERT(info == GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// adjust label matrices
	//--------------------------------------------------------------------------

	for(int i = 0; i < labels_count; i++) {
		GrB_Matrix M = RG_MATRIX_M(Graph_GetLabelMatrix(g, i));
		info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, M, PC, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	for(int i = 0; i < reltype_count; i++) {
		GrB_Matrix M = RG_MATRIX_M(Graph_GetRelationMatrix(g, i, false));
		info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_UINT64, M, PC, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	//--------------------------------------------------------------------------
	// adjust adjacency matrx
	//--------------------------------------------------------------------------

	M = RG_MATRIX_M(Graph_GetAdjacencyMatrix(g, false));
	info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, M, PC, NULL);
	ASSERT(info == GrB_SUCCESS);
	printf("new ADJ matrix\n");
	GxB_print(M, GxB_SHORT);

	//--------------------------------------------------------------------------
	// adjust node labels matrix
	//--------------------------------------------------------------------------

	M = RG_MATRIX_M(Graph_GetNodeLabelMatrix(g));
	info = GrB_mxm(M, NULL, NULL, GxB_ANY_PAIR_BOOL, M, PC, NULL);
	ASSERT(info == GrB_SUCCESS);

	// trim empty blocks
	int blocks_removed = DataBlock_Trim(nodes);
	printf("blocks_removed: %d\n", blocks_removed);

	// clean up
	GrB_free(&PR);
	DataBlockIterator_Free(it);

	return 0;
}

// GRAPH.VACUUM <key>
int Graph_Vacuum
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	if(argc < 2) {
		return RedisModule_WrongArity(ctx);
	}

	// get graph context
	RedisModuleString *graph_name = argv[1];
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, false, false);
	if(gc == NULL) {
		// missing graph context
		return REDISMODULE_OK;
	}

	// get graph and underline datablocks
	Graph *g = gc->g;
	uint64_t vacuumed = _Vacuum(g);

	RedisModule_ReplyWithSimpleString(ctx, "OK");

	return REDISMODULE_OK;
}

