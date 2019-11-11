/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./detect_cycle.h"
#include "rax.h"
#include "../util/arr.h"
#include "../graph/entities/qg_node.h"
#include "../GraphBLASExt/GxB_Clone.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

GrB_Matrix GxB_IdentityMatrix(GrB_Index dimension) {
	GrB_Matrix identity;
	// TODO: get identity of type.
	GrB_Info res = GrB_Matrix_new(&identity, GrB_BOOL, dimension, dimension);
	assert(res == GrB_SUCCESS);
	for(uint i = 0; i < dimension; i++) {
		res = GrB_Matrix_setElement_BOOL(identity, true, i, i);
		assert(res == GrB_SUCCESS);
	}

	return identity;
}

bool AcyclicGraph(const QueryGraph *qg) {
	assert(qg);

	bool cycle = false; // Return value.

	// Give an ID for each node, abuse of `labelID`.
	uint node_count = QueryGraph_NodeCount(qg);
	uint edge_count = QueryGraph_EdgeCount(qg);

	GrB_Info res;
	GrB_Matrix m;   // Matrix representation of QueryGraph.
	GrB_Matrix c;   // Intermidate matrix, c = m^i.
	GrB_Matrix t;   // Temporary matrix, t = c .* i. elementwise boolean multiplication.
	GrB_Matrix identity;
	GrB_Descriptor desc;

	res = GrB_Descriptor_new(&desc);
	assert(res == GrB_SUCCESS);
	res = GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
	assert(res == GrB_SUCCESS);

	// Build matrix representation of query graph.
	m = QueryGraph_MatrixRepresentation(qg);
	res = GxB_MatrixClone(m, &c);
	assert(res == GrB_SUCCESS);
	identity = GxB_IdentityMatrix(node_count);
	res = GrB_Matrix_new(&t, GrB_BOOL, node_count, node_count);
	assert(res == GrB_SUCCESS);

	/* Perform traversals, stop when:
	* 1. Node i manged to reach itself, x[i,i] is set (cycle detected).
	* 2. After edge_count multiplications, no cycles. */
	for(uint i = 0; i < edge_count; i++) {
		// c = c * m.
		res = GrB_mxm(c, GrB_NULL, GrB_NULL, GxB_LOR_LAND_BOOL, c, m, GrB_NULL);
		assert(res == GrB_SUCCESS);

		/* Check if C[i,i] is set.
		 * t = c<identity> */
		res = GrB_transpose(t, identity, GrB_NULL, c, desc);
		assert(res == GrB_SUCCESS);
		/* How many entries are there in `t`?
		 * if there are any entires in `t` this means node `k` was able to reach itself cycle! */
		GrB_Index nvals = 0;
		res = GrB_Matrix_nvals(&nvals, t);
		assert(res == GrB_SUCCESS);
		if(nvals != 0) {
			cycle = true;
			break;
		}
	}

	// Cleanup.
	GrB_free(&m);
	GrB_free(&c);
	GrB_free(&t);
	GrB_free(&identity);

	return cycle;
}
