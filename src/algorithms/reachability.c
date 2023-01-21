/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "reachability.h"
#include "GraphBLAS.h"

// returns true if there's a path connecting 'src' to 'dest'
bool reachable
(
	NodeID src,     // source node to traverse from
	NodeID dest,    // destination node to reach
	const Graph *g  // graph to traverse
) {
	// validations
	
	// both node IDs must be valid
	ASSERT(src != INVALID_ENTITY_ID);
	ASSERT(dest != INVALID_ENTITY_ID);

	// both node IDs should be within bounds
	GrB_Index n = Graph_RequiredMatrixDim(g);
	ASSERT(src < n);
	ASSERT(dest < n);

	// quick return if src == dest
	if(src == dest) {
		return true;
	}

	bool       x;
	GrB_Vector Q;
	GrB_Vector M;
	GrB_Matrix A;
	GrB_Info   info;
	GrB_Index  nvals;

	//--------------------------------------------------------------------------
	// get graph GrB adjacency matrix
	//--------------------------------------------------------------------------

	RG_Matrix _A = Graph_GetAdjacencyMatrix(g, false);
	// flush _A in case it contains any pending changes
	// note: this can be expensive!
	//if(RG_Matrix_isDirty(_A)) {
		info = RG_Matrix_wait(_A, true);
		ASSERT(info == GrB_SUCCESS);
	//}
	A = RG_MATRIX_M(_A);

	//--------------------------------------------------------------------------
	// create q vector
	//--------------------------------------------------------------------------

	info = GrB_Vector_new(&Q, GrB_BOOL, n);
	ASSERT(info == GrB_SUCCESS);

	// set traversal starting point
	GrB_Vector_setElement_BOOL(Q, true, src);

	//--------------------------------------------------------------------------
	// create mask
	//--------------------------------------------------------------------------

	info = GrB_Vector_new(&M, GrB_BOOL, n);
	ASSERT(info == GrB_SUCCESS);

	// as long as:
	// destination node wasn't reached
	// there are nodes to explore

	//GxB_print(Q, GxB_COMPLETE_VERBOSE);
	//GxB_print(M, GxB_COMPLETE_VERBOSE);
	//GxB_print(A, GxB_COMPLETE_VERBOSE);

	bool reached = false;
	while(true) {
		//----------------------------------------------------------------------
		// get direct neighbors
		//----------------------------------------------------------------------

		info = GrB_vxm(
				Q,                  // input/output vector for results
				M,                  // optional mask for w, unused if NULL
				NULL,               // optional accum for z=accum(w,t)
				GxB_ANY_PAIR_BOOL,  // defines '+' and '*' for u'*A
				Q,                  // first input:  vector u
				A,                  // second input: matrix A
				GrB_DESC_RSC        // descriptor for w, mask, and A
		);
		ASSERT(info == GrB_SUCCESS);
		//GxB_print(Q, GxB_COMPLETE_VERBOSE);

		//----------------------------------------------------------------------
		// see if we've reached any new nodes
		//----------------------------------------------------------------------

		info = GrB_Vector_nvals(&nvals, Q);
		ASSERT(info == GrB_SUCCESS);

		if(nvals == 0) {
			// no new nodes been reached
			break;
		}

		//----------------------------------------------------------------------
		// check if destination been reached
		//----------------------------------------------------------------------

		info = GrB_Vector_extractElement_BOOL(&x, Q, dest);
		reached = (info == GrB_SUCCESS);
		if(reached == true) {
			// destination node is reachable
			break;
		}

		//----------------------------------------------------------------------
		// update mask
		//----------------------------------------------------------------------

		info = GrB_Vector_assign(
				M,          // input/output matrix for results
				Q,          // optional mask for w, unused if NULL
				NULL,       // optional accum for z=accum(w(I),t)
				Q,          // first input:  vector u
				GrB_ALL,    // row indices
				n,          // number of row indices
				GrB_DESC_S  // descriptor for w and mask
		);
		ASSERT(info == GrB_SUCCESS);
		//GxB_print(M, GxB_COMPLETE_VERBOSE);
	}

	return reached;
}

