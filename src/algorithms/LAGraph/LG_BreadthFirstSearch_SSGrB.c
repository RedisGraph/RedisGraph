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
#include "LAGraph_bfs_pushpull.h"

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

	GrB_Info info ;
	GrB_Vector  q   =  NULL  ;  //  the current frontier
	GrB_Vector  w   =  NULL  ;  //  to compute work remaining
	GrB_Vector  pi  =  NULL  ;  //  parent vector
	GrB_Vector  v   =  NULL  ;  //  level vector

	UNUSED(info);

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
	GrB_OK (GrB_Matrix_nrows (&n, A)) ;

	GrB_OK (GrB_Matrix_nvals (&nvals, A)) ;

	GrB_Vector Degree ;
	LAGraph_Property_RowDegree(A, &Degree) ;

	// direction-optimization requires AT and Degree
	bool push_pull = (AT != NULL) && (Degree != NULL) ;

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
		GrB_OK (GrB_Vector_new (&pi, int_type, n)) ;
		GrB_OK (GxB_set (pi, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
		// pi (src) = src denotes the root of the BFS tree
		GrB_OK (GrB_Vector_setElement (pi, src, src)) ;

		// create a sparse integer vector q, and set q(src) = src
		GrB_OK (GrB_Vector_new (&q, int_type, n)) ;
		GrB_OK (GrB_Vector_setElement (q, src, src)) ;
	}
	else
	{
		// only the level is needed
		semiring = GrB_LOR_LAND_SEMIRING_BOOL ;

		// create a sparse boolean vector q, and set q(src) = true
		GrB_OK (GrB_Vector_new (&q, GrB_BOOL, n)) ;
		GrB_OK (GrB_Vector_setElement (q, true, src)) ;
	}

	if (compute_level)
	{
		// create the level vector. v(i) is the level of node i
		// v (src) = 0 denotes the source node

		GrB_OK (GrB_Vector_new (&v, int_type, n)) ;
		GrB_OK (GxB_set (v, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
		GrB_OK (GrB_Vector_setElement (v, 0, src)) ;
	}

	if (push_pull)
	{
		// workspace for computing work remaining
		GrB_OK (GrB_Vector_new (&w, GrB_INT64, n)) ;
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
	GrB_Index last_nq = 0 ;
	int64_t edges_unexplored = nvals ;
	bool any_pull = false ;     // true if any pull phase has been done

	// {!mask} is the set of unvisited nodes
	GrB_Vector mask = (compute_parent) ? pi : v ;

	// create a scalar to hold the destination value
	GrB_Index dest_val ;

	for (int64_t nvisited = 1, k = 1 ; nvisited < n ; nvisited += nq, k++)
	{

		//----------------------------------------------------------------------
		// select push vs pull
		//----------------------------------------------------------------------

		if (push_pull)
		{
			if (do_push)
			{
				// check for switch from push to pull
				bool growing = nq > last_nq ;
				bool switch_to_pull = false ;
				if (edges_unexplored < n)
				{
					// very little of the graph is left; disable the pull
					push_pull = false ;
				}
				else if (any_pull)
				{
					// once any pull phase has been done, the # of edges in the
					// frontier has no longer been tracked.  But now the BFS
					// has switched back to push, and we're checking for yet
					// another switch to pull.  This switch is unlikely, so
					// just keep track of the size of the frontier, and switch
					// if it starts growing again and is getting big.
					switch_to_pull = (growing && nq > n_over_beta1) ;
				}
				else
				{
					// update the # of unexplored edges
					// w<q>=Degree
					// w(i) = outdegree of node i if node i is in the queue
					GrB_OK (GrB_assign (w, q, NULL, Degree, GrB_ALL, n,
						GrB_DESC_RS)) ;
					// edges_in_frontier = sum (w) = # of edges incident on all
					// nodes in the current frontier
					int64_t edges_in_frontier = 0 ;
					GrB_OK (GrB_reduce (&edges_in_frontier, NULL,
						GrB_PLUS_MONOID_INT64, w, NULL)) ;
					edges_unexplored -= edges_in_frontier ;
					switch_to_pull = growing &&
						(edges_in_frontier > (edges_unexplored / alpha)) ;
				}
				if (switch_to_pull)
				{
					// switch from push to pull
					do_push = false ;
				}
			}
			else
			{
				// check for switch from pull to push
				bool shrinking = nq < last_nq ;
				if (shrinking && (nq <= n_over_beta2))
				{
					// switch from pull to push
					do_push = true ;
				}
			}
			any_pull = any_pull || (!do_push) ;
		}

		//----------------------------------------------------------------------
		// q = kth level of the BFS
		//----------------------------------------------------------------------

		// mask is pi if computing parent, v if computing just level
		if (do_push)
		{
			// q'{!mask} = q'*A
			GrB_OK (GrB_vxm (q, mask, NULL, semiring, q, A, GrB_DESC_RSC)) ;
		}
		else
		{
			// q{!mask} = AT*q
			GrB_OK (GrB_mxv (q, mask, NULL, semiring, AT, q, GrB_DESC_RSC)) ;
		}

		//----------------------------------------------------------------------
		// done if q is empty
		//----------------------------------------------------------------------

		last_nq = nq ;
		GrB_OK (GrB_Vector_nvals (&nq, q)) ;
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
			GrB_OK (GrB_assign (pi, q, NULL, q, GrB_ALL, n, GrB_DESC_S)) ;
		}
		if (compute_level)
		{
			// v{q} = k, the kth level of the BFS
			GrB_OK (GrB_assign (v, q, NULL, k, GrB_ALL, n, GrB_DESC_S)) ;
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

	GrB_free (&w) ;
	GrB_free (&q) ;
	GrB_free (&Degree) ;

	return (0) ;
}
