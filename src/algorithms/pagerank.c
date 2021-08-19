//------------------------------------------------------------------------------
// LAGraph_pagerank: pagerank using a real semiring
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS
    Copyright 2019 LAGraph Contributors.
    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).
    All Rights Reserved.
    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.
    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).
    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.
*/

#include "pagerank.h"
#include "../util/rmalloc.h"
#include <assert.h>

//------------------------------------------------------------------------------
// scalar operators
//------------------------------------------------------------------------------

#define DAMPING 0.85

void fdiff(void *z, const void *x, const void *y) {
	float delta = (* ((float *) x)) - (* ((float *) y)) ;
	(*((float *) z)) = delta * delta ;
}

//------------------------------------------------------------------------------
// comparison function for qsort
//------------------------------------------------------------------------------

int compar(const void *x, const void *y) {
	LAGraph_PageRank *a = (LAGraph_PageRank *) x ;
	LAGraph_PageRank *b = (LAGraph_PageRank *) y ;

	// sort by pagerank in descending order
	if(a->pagerank > b->pagerank) {
		return (-1) ;
	} else if(a->pagerank == b->pagerank) {
		return (0) ;
	} else {
		return (1) ;
	}
}

//------------------------------------------------------------------------------
// LAGraph_pagerank: compute the pagerank of all nodes in a graph
//------------------------------------------------------------------------------

GrB_Info Pagerank               // GrB_SUCCESS or error condition
(
	LAGraph_PageRank **Phandle, // output: array of LAGraph_PageRank structs
	GrB_Matrix A,               // binary input graph, not modified
	int itermax,                // max number of iterations
	double tol,                 // stop when norm (r-rnew,2) < tol
	int *iters                  // number of iterations taken
) {

	//--------------------------------------------------------------------------
	// initializations
	//--------------------------------------------------------------------------

	GrB_Info info ;
	float rsum ;
	float *X = NULL ;
	LAGraph_PageRank *P = NULL ;
	GrB_BinaryOp op_diff = NULL ;
	GrB_Index n, nvals, *I = NULL ;
	GrB_Vector r = NULL, t = NULL, d = NULL ;
	GrB_Matrix C = NULL, D = NULL, T = NULL ;

	assert(Phandle);
	(*Phandle) = NULL ;

	// n = size (A,1) ;         // number of nodes
	assert(GrB_Matrix_nrows(&n, A) == GrB_SUCCESS) ;
	if(n == 0) return (GrB_SUCCESS) ;

	// teleport = (1 - 0.85) / n
	float one = 1.0 ;
	float teleport = (one - DAMPING) / ((float) n) ;

	// r (i) = 1/n for all nodes i
	float x = 1.0 / ((float) n) ;
	assert(GrB_Vector_new(&r, GrB_FP32, n) == GrB_SUCCESS) ;
	assert(GrB_assign(r, NULL, NULL, x, GrB_ALL, n, NULL) == GrB_SUCCESS) ;

	// d (i) = out deg of node i
	assert(GrB_Vector_new(&d, GrB_FP32, n) == GrB_SUCCESS) ;
	assert(GrB_reduce(d, NULL, NULL, GrB_PLUS_FP32, A, NULL) == GrB_SUCCESS) ;
	// GxB_print (d, 3) ;

	// D = (1/diag (d)) * DAMPING
	bool iso ;
	bool jumbled ;
	GrB_Type type ;
    GrB_Index vi_size;
    GrB_Index vx_size;

	assert(GxB_Vector_export_CSC(&d, &type, &n, &I, (void **)(&X), &vi_size,
				&vx_size, &iso, &nvals, &jumbled, NULL) == GrB_SUCCESS) ;

	for(int64_t k = 0 ; k < nvals ; k++) X [k] = DAMPING / X [k] ;
	assert(GrB_Matrix_new(&D, GrB_FP32, n, n) == GrB_SUCCESS) ;
	assert(GrB_Matrix_build(D, I, I, X, nvals, GrB_PLUS_FP32) == GrB_SUCCESS) ;
	rm_free(I) ;
	rm_free(X) ;

	// C = diagonal matrix with all zeros on the diagonal.  This ensures that
	// the vectors r and t remain dense, which is faster, and is required
	// for the t += teleport_scalar step.
	assert(GrB_Matrix_new(&C, GrB_FP32, n, n) == GrB_SUCCESS) ;
	// GxB_set (C, GxB_HYPER, GxB_ALWAYS_HYPER) ;

	for(int64_t k = 0 ; k < n ; k++) {
		// C(k,k) = 0
		assert(GrB_Matrix_setElement(C, (float) 0, k, k) == GrB_SUCCESS) ;
	}

	// make sure D is diagonal
	assert(GrB_eWiseAdd(D, NULL, NULL, GrB_PLUS_FP32, D, C, NULL) == GrB_SUCCESS) ;

	// use GrB_mxv for t=C*r below
	// C = C+(D*A)' = C+A'*D'  : using the transpose of C, and C*r below

	// T = D*A
	assert(GrB_Matrix_new(&T, GrB_FP32, n, n) == GrB_SUCCESS) ;
	assert(GrB_mxm(T, NULL, NULL, GxB_PLUS_TIMES_FP32, D, A, NULL) == GrB_SUCCESS) ;

	// C = C+T'
	assert(GrB_transpose(C, NULL, GrB_PLUS_FP32, T, NULL) == GrB_SUCCESS) ;

	assert(GrB_free(&T) == GrB_SUCCESS) ;
	assert(GrB_free(&D) == GrB_SUCCESS) ;

	// create operator
	assert(GrB_BinaryOp_new(&op_diff, fdiff, GrB_FP32, GrB_FP32, GrB_FP32) == GrB_SUCCESS) ;

	float ftol = tol * tol ; // use tol^2 so sqrt(rdiff) not needed
	float rdiff = 1 ;       // so first iteration is always done

	assert(GrB_Vector_new(&t, GrB_FP32, n) == GrB_SUCCESS) ;

	//--------------------------------------------------------------------------
	// iterate to compute the pagerank of each node
	//--------------------------------------------------------------------------

	for((*iters) = 0 ; (*iters) < itermax && rdiff > ftol ; (*iters)++) {

		//----------------------------------------------------------------------
		// t = (r*C or C*r) + (teleport * sum (r)) ;
		//----------------------------------------------------------------------
		// GxB_print (r, 2) ;

		assert(GrB_reduce(&rsum, NULL, GxB_PLUS_FP32_MONOID, r, NULL) == GrB_SUCCESS) ;

		// t = C*r
		// using the transpose of A, scaled (dot product)
		assert(GrB_mxv(t, NULL, NULL, GxB_PLUS_TIMES_FP32, C, r, NULL) == GrB_SUCCESS) ;

		// t += teleport_scalar ;
		float teleport_scalar = teleport * rsum ;
		assert(GrB_assign(t, NULL, GrB_PLUS_FP32, teleport_scalar, GrB_ALL, n, NULL) == GrB_SUCCESS) ;
		//----------------------------------------------------------------------
		// rdiff = sum ((r-t).^2)
		//----------------------------------------------------------------------

		assert(GrB_eWiseAdd(r, NULL, NULL, op_diff, r, t, NULL) == GrB_SUCCESS) ;
		assert(GrB_reduce(&rdiff, NULL, GxB_PLUS_FP32_MONOID, r, NULL) == GrB_SUCCESS) ;

		//----------------------------------------------------------------------
		// swap r and t
		//----------------------------------------------------------------------

		GrB_Vector temp = r ;
		r = t ;
		t = temp ;
	}

	assert(GrB_free(&C) == GrB_SUCCESS) ;
	assert(GrB_free(&t) == GrB_SUCCESS) ;

	//--------------------------------------------------------------------------
	// scale the result
	//--------------------------------------------------------------------------

	// rsum = sum (r)
	assert(GrB_reduce(&rsum, NULL, GxB_PLUS_FP32_MONOID, r, NULL) == GrB_SUCCESS) ;

	// r = r / rsum
	assert(GrB_Vector_assign_FP32(r, NULL, GrB_TIMES_FP32, 1 / rsum, GrB_ALL, n, NULL) == GrB_SUCCESS) ;

	//--------------------------------------------------------------------------
	// sort the nodes by pagerank
	//--------------------------------------------------------------------------

	// [r,irank] = sort (r, 'descend') ;

	// extract the contents of r
	assert(GxB_Vector_export_CSC(&r, &type, &n, &I, (void **)(&X), &vi_size,
				&vx_size, &iso, &nvals, &jumbled, NULL) == GrB_SUCCESS) ;

	// this will always be true since r is dense, but check anyway:
	if(nvals != n) return (GrB_PANIC) ;

	// P = struct (X,I)
	P = rm_malloc(n * sizeof(LAGraph_PageRank)) ;
	assert(P != NULL);

	for(int64_t k = 0 ; k < nvals ; k++) {
		// The kth ranked page is P[k].page (with k=0 being the highest rank),
		// and its pagerank is P[k].pagerank.
		P [k].pagerank = X [k] ;
		// I [k] == k will be true for SuiteSparse:GraphBLAS but in general I
		// can be returned in any order, so use I [k] instead of k, for other
		// GraphBLAS implementations.
		P [k].page = k ;
	}

	// qsort (P) in descending order
	qsort(P, n, sizeof(LAGraph_PageRank), compar) ;

	//--------------------------------------------------------------------------
	// return result
	//--------------------------------------------------------------------------

	(*Phandle) = P ;

	// Clean up.
	rm_free(I) ;
	rm_free(X) ;
	GrB_free(&T) ;
	GrB_free(&D) ;
	GrB_free(&C) ;
	GrB_free(&r) ;
	GrB_free(&t) ;
	GrB_free(&d) ;
	GrB_free(&op_diff) ;

	return (GrB_SUCCESS) ;
}
