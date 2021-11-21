//------------------------------------------------------------------------------
// LG_BreadthFirstSearch_SSGrB:  BFS using Suitesparse extensions
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// See additional acknowledgments in the LICENSE file,
// or contact permission@sei.cmu.edu for the full terms.

//------------------------------------------------------------------------------

// References:
//
// Carl Yang, Aydin Buluc, and John D. Owens. 2018. Implementing Push-Pull
// Efficiently in GraphBLAS. In Proceedings of the 47th International
// Conference on Parallel Processing (ICPP 2018). ACM, New York, NY, USA,
// Article 89, 11 pages. DOI: https://doi.org/10.1145/3225058.3225122
//
// Scott Beamer, Krste Asanovic and David A. Patterson, The GAP Benchmark
// Suite, http://arxiv.org/abs/1508.03619, 2015.  http://gap.cs.berkeley.edu/

#include "RG.h"
#include "GraphBLAS.h"

#define LAGraph_FREE_WORK   \
{                           \
	GrB_free (&w) ;         \
	GrB_free (&q) ;         \
}

#define LAGraph_FREE_ALL    \
{                           \
	LAGraph_FREE_WORK ;     \
	GrB_free (&pi) ;        \
	GrB_free (&v) ;         \
}

#define GrB_TRY(res)             \
{                                \
	info = res ;                 \
	ASSERT(info == GrB_SUCCESS) ; \
}

//****************************************************************************
int LG_BreadthFirstSearch_SSGrB
(
	GrB_Vector    *level,
	GrB_Vector    *parent,
	GrB_Matrix    A,
	GrB_Matrix    AT,
	GrB_Index     src,
	GrB_Index     *dest,
	GrB_Index     max_level
)
{
	//--------------------------------------------------------------------------
	// check inputs
	//--------------------------------------------------------------------------

	GrB_Vector q = NULL ;           // the current frontier
	GrB_Vector w = NULL ;           // to compute work remaining
	GrB_Vector pi = NULL ;          // parent vector
	GrB_Vector v = NULL ;           // level vector
	GrB_Info info;

	bool compute_level  = (level != NULL) ;
	bool compute_parent = (parent != NULL) ;
	if (compute_level ) (*level ) = NULL ;
	if (compute_parent) (*parent) = NULL ;

	if (!(compute_level || compute_parent))
	{
		// nothing to do
		return (0) ;
	}

	//--------------------------------------------------------------------------
	// get the problem size and properties
	//--------------------------------------------------------------------------
	GrB_Index n, nvals ;
	GrB_TRY (GrB_Matrix_nrows (&n, A)) ;

	GrB_TRY (GrB_Matrix_nvals (&nvals, A)) ;

	// determine the semiring type
	GrB_Type int_type = (n > INT32_MAX) ? GrB_INT64 : GrB_INT32 ;
	GrB_Semiring semiring ;

	if (compute_parent)
	{
		// use the ANY_SECONDI_INT* semiring: either 32 or 64-bit depending on
		// the # of nodes in the graph.
		semiring = (n > INT32_MAX) ?
			GxB_ANY_SECONDI_INT64 : GxB_ANY_SECONDI_INT32 ;

		// create the parent vector.  pi(i) is the parent id of node i
		GrB_TRY (GrB_Vector_new (&pi, int_type, n)) ;
		GrB_TRY (GxB_set (pi, GxB_SPARSITY_CONTROL, GxB_BITMAP + GxB_FULL)) ;
		// pi (src) = src denotes the root of the BFS tree
		GrB_TRY (GrB_Vector_setElement (pi, src, src)) ;

		// create a sparse integer vector q, and set q(src) = src
		GrB_TRY (GrB_Vector_new (&q, int_type, n)) ;
		GrB_TRY (GrB_Vector_setElement (q, src, src)) ;
	}
	else
	{
		// only the level is needed
		semiring = GrB_LOR_LAND_SEMIRING_BOOL ;

		// create a sparse boolean vector q, and set q(src) = true
		GrB_TRY (GrB_Vector_new (&q, GrB_BOOL, n)) ;
		GrB_TRY (GrB_Vector_setElement (q, true, src)) ;
	}

	if (compute_level)
	{
		// create the level vector. v(i) is the level of node i
		// v (src) = 0 denotes the source node
		GrB_TRY (GrB_Vector_new (&v, int_type, n)) ;
		GrB_TRY (GxB_set (v, GxB_SPARSITY_CONTROL, GxB_BITMAP + GxB_FULL)) ;
		GrB_TRY (GrB_Vector_setElement (v, 0, src)) ;
	}

	GrB_Index nq = 1 ;          // number of nodes in the current level
	double alpha = 8.0 ;
	double beta1 = 8.0 ;
	double beta2 = 512.0 ;
	int64_t n_over_beta1 = (int64_t) (((double) n) / beta1) ;
	int64_t n_over_beta2 = (int64_t) (((double) n) / beta2) ;

	//--------------------------------------------------------------------------
	// BFS traversal and label the nodes
	//--------------------------------------------------------------------------

	bool do_push = true ;       // start with push
	int64_t edges_unexplored = nvals ;
	bool any_pull = false ;     // true if any pull phase has been done

	// {!mask} is the set of unvisited nodes
	GrB_Vector mask = (compute_parent) ? pi : v ;

	// create a scalar to hold the destination value
	GrB_Index dest_val ;

	for (int64_t nvisited = 1, k = 1 ; nvisited < n ; nvisited += nq, k++)
	{
		//----------------------------------------------------------------------
		// q = kth level of the BFS
		//----------------------------------------------------------------------

		int sparsity = do_push ? GxB_SPARSE : GxB_BITMAP ;
		GrB_TRY (GxB_set (q, GxB_SPARSITY_CONTROL, sparsity)) ;

		// mask is pi if computing parent, v if computing just level
		if (do_push)
		{
			// q'{!mask} = q'*A
			GrB_TRY (GrB_vxm (q, mask, NULL, semiring, q, A, GrB_DESC_RSC)) ;
		}
		else
		{
			// q{!mask} = AT*q
			GrB_TRY (GrB_mxv (q, mask, NULL, semiring, AT, q, GrB_DESC_RSC)) ;
		}

		//----------------------------------------------------------------------
		// done if q is empty
		//----------------------------------------------------------------------

		GrB_TRY (GrB_Vector_nvals (&nq, q)) ;
		if (nq == 0)
		{
			break ;
		}

		//----------------------------------------------------------------------
		// assign parents/levels
		//----------------------------------------------------------------------

		if (compute_parent)
		{
			// q(i) currently contains the parent id of node i in tree.
			// pi{q} = q
			GrB_TRY (GrB_assign (pi, q, NULL, q, GrB_ALL, n, GrB_DESC_S)) ;
		}
		if (compute_level)
		{
			// v{q} = k, the kth level of the BFS
			GrB_TRY (GrB_assign (v, q, NULL, k, GrB_ALL, n, GrB_DESC_S)) ;
		}
		// check if destination has been reached, if one is provided
		if(dest) {
			GrB_Info res = GrB_Vector_extractElement(&dest_val, q, *dest) ;
			if(res != GrB_NO_VALUE) break ;
		}
		// check if max level has been reached, if one is provided
		if(max_level != 0 && k >= max_level) break;
	}

	//--------------------------------------------------------------------------
	// free workspace and return result
	//--------------------------------------------------------------------------

	if (compute_parent) (*parent) = pi ;
	if (compute_level ) (*level ) = v ;
	LAGraph_FREE_WORK ;
	return (0) ;
}
