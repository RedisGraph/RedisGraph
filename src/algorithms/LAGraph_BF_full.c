//------------------------------------------------------------------------------
// LAGraph_BF_full1.c: Bellman-Ford single-source shortest paths, returns tree,
// while diagonal of input matrix A needs not to be explicit 0
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

//------------------------------------------------------------------------------

// LAGraph_BF_full1: Bellman-Ford single source shortest paths, returning both
// the path lengths and the shortest-path tree.  contributed by Jinhao Chen and
// Tim Davis, Texas A&M.

// LAGraph_BF_full performs a Bellman-Ford to find out shortest path, parent
// nodes along the path and the hops (number of edges) in the path from given
// source vertex s in the range of [0, n) on graph given as matrix A with size
// n*n. The sparse matrix A has entry A(i, j) if there is an edge from vertex i
// to vertex j with weight w, then A(i, j) = w.

// TODO: think about the return values

// LAGraph_BF_full1 returns GrB_SUCCESS regardless of existence of negative-
// weight cycle. However, the GrB_Vector d(k), pi(k) and h(k)  (i.e.,
// *pd_output, *ppi_output and *ph_output respectively) will be NULL when
// negative-weight cycle detected. Otherwise, the vector d has d(k) as the
// shortest distance from s to k. pi(k) = p+1, where p is the parent node of
// k-th node in the shortest path. In particular, pi(s) = 0. h(k) = hop(s, k),
// the number of edges from s to k in the shortest path.

//------------------------------------------------------------------------------
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../util/rmalloc.h"
#include "LAGraph/LAGraph.h"

#define FREE_ALL               \
{                                      \
    GrB_free(&d);                      \
    GrB_free(&dmasked);                \
    GrB_free(&dless);                  \
    GrB_free(&Atmp);                   \
    GrB_free(&BF_Tuple3);              \
    GrB_free(&BF_lMIN_Tuple3);         \
    GrB_free(&BF_PLUSrhs_Tuple3);      \
    GrB_free(&BF_Identity_Tuple3);     \
    GrB_free(&BF_LT_Tuple3);           \
    GrB_free(&BF_lMIN_Tuple3_Monoid);  \
    GrB_free(&BF_lMIN_PLUSrhs_Tuple3); \
    rm_free (I);                       \
    rm_free (J);                       \
    rm_free (w);                       \
    rm_free (W);                       \
    rm_free (h);                       \
    rm_free (pi);                      \
}

//------------------------------------------------------------------------------
// data type for each entry of the adjacent matrix A and "distance" vector d;
// <INFINITY,INFINITY,INFINITY> corresponds to nonexistence of a path, and
// the value  <0, 0, NULL> corresponds to a path from a vertex to itself
//------------------------------------------------------------------------------
typedef struct {
	double w;    // w  corresponds to a path weight.
	GrB_Index h; // h  corresponds to a path size or number of hops.
	GrB_Index pi;// pi corresponds to the penultimate vertex along a path.
	// vertex indexed as 1, 2, 3, ... , V, and pi = 0 (as nil)
	// for u=v, and pi = UINT64_MAX (as inf) for (u,v) not in E
}
BF_Tuple3_struct;

//------------------------------------------------------------------------------
// 2 binary functions, z=f(x,y), where Tuple3xTuple3 -> Tuple3
//------------------------------------------------------------------------------
void BF_lMIN
(
	void *z,
	const void *x,
	const void *y
) {
	BF_Tuple3_struct *_z = z;
	const BF_Tuple3_struct *_x = x;
	const BF_Tuple3_struct *_y = y;

	if(_x->w < _y->w
	   || (_x->w == _y->w && _x->h < _y->h)
	   || (_x->w == _y->w && _x->h == _y->h && _x->pi < _y->pi)) {
		if(_z != _x) {
			*_z = *_x;
		}
	} else {
		*_z = *_y;
	}
}

void BF_PLUSrhs
(
	void *z,
	const void *x,
	const void *y
) {
	BF_Tuple3_struct *_z = z;
	const BF_Tuple3_struct *_x = x;
	const BF_Tuple3_struct *_y = y;

	_z->w = _x->w + _y->w;
	_z->h = _x->h + _y->h;
	if(_x->pi != UINT64_MAX && _y->pi != 0) {
		_z->pi = _y->pi;
	} else {
		_z->pi = _x->pi;
	}
}

void BF_Identity
(
	BF_Tuple3_struct *z,
	const BF_Tuple3_struct *x
) {
	*z = *x;
}

void BF_LT
(
	void *z,
	const void *x,
	const void *y

) {
	bool *_z = z;
	const BF_Tuple3_struct *_x = x;
	const BF_Tuple3_struct *_y = y;

	if(_x->w < _y->w
	   || (_x->w == _y->w && _x->h < _y->h)
	   || (_x->w == _y->w && _x->h == _y->h && _x->pi < _y->pi)) {
		*_z = true;
	} else {
		*_z = false;
	}
}
// Given a n-by-n adjacency matrix A and a source vertex s.
// If there is no negative-weight cycle reachable from s, return the distances
// of shortest paths from s and parents along the paths as vector d. Otherwise,
// returns d=NULL if there is a negtive-weight cycle.
// pd_output is pointer to a GrB_Vector, where the i-th entry is d(s,i), the
//   sum of edges length in the shortest path
// ppi_output is pointer to a GrB_Vector, where the i-th entry is pi(i), the
//   parent of i-th vertex in the shortest path
// ph_output is pointer to a GrB_Vector, where the i-th entry is h(s,i), the
//   number of edges from s to i in the shortest path
// A has weights on corresponding entries of edges
// s is given index for source vertex
GrB_Info LAGraph_BF_full
(
	GrB_Vector *pd_output,      //the pointer to the vector of distance
	GrB_Vector *ppi_output,     //the pointer to the vector of parent
	GrB_Vector *ph_output,      //the pointer to the vector of hops
	const GrB_Matrix A,         //matrix for the graph
	const GrB_Index s           //given index of the source
) {
	GrB_Info info;
	// tmp vector to store distance vector after n (i.e., V) loops
	GrB_Vector d = NULL, dmasked = NULL, dless = NULL;
	GrB_Matrix Atmp = NULL;
	GrB_Type BF_Tuple3;

	GrB_BinaryOp BF_lMIN_Tuple3;
	GrB_BinaryOp BF_PLUSrhs_Tuple3;
	GrB_UnaryOp BF_Identity_Tuple3;
	GrB_BinaryOp BF_LT_Tuple3;

	GrB_Monoid BF_lMIN_Tuple3_Monoid;
	GrB_Semiring BF_lMIN_PLUSrhs_Tuple3;

	GrB_Index nrows, ncols, n, nz;  // n = # of row/col, nz = # of nnz in graph
	GrB_Index *I = NULL, *J = NULL; // for col/row indices of entries from A
	GrB_Index *h = NULL, *pi = NULL;
	double *w = NULL;
	BF_Tuple3_struct *W = NULL;

	if(A == NULL || pd_output == NULL ||
	   ppi_output == NULL || ph_output == NULL) {
		// required argument is missing
		return GrB_NULL_POINTER;
	}

	*pd_output  = NULL;
	*ppi_output = NULL;
	*ph_output  = NULL;
	GrB_Matrix_nrows(&nrows, A) ;
	GrB_Matrix_ncols(&ncols, A) ;
	GrB_Matrix_nvals(&nz, A);
	if(nrows != ncols) {
		// A must be square
		return GrB_INVALID_VALUE;
	}
	n = nrows;

	if(s >= n || s < 0) {
		return GrB_INVALID_VALUE;
	}
	//--------------------------------------------------------------------------
	// create all GrB_Type GrB_BinaryOp GrB_Monoid and GrB_Semiring
	//--------------------------------------------------------------------------
	// GrB_Type
	GrB_Type_new(&BF_Tuple3, sizeof(BF_Tuple3_struct));

	// GrB_BinaryOp
	GrB_UnaryOp_new(&BF_Identity_Tuple3, (void *)(&BF_Identity), BF_Tuple3, BF_Tuple3);
	GrB_BinaryOp_new(&BF_LT_Tuple3, &BF_LT, GrB_BOOL, BF_Tuple3, BF_Tuple3);
	GrB_BinaryOp_new(&BF_lMIN_Tuple3, &BF_lMIN, BF_Tuple3, BF_Tuple3, BF_Tuple3);
	GrB_BinaryOp_new(&BF_PLUSrhs_Tuple3, &BF_PLUSrhs, BF_Tuple3, BF_Tuple3, BF_Tuple3);

	// GrB_Monoid
	BF_Tuple3_struct BF_identity = (BF_Tuple3_struct) {
		.w = INFINITY,
		.h = UINT64_MAX, .pi = UINT64_MAX
	};
	GrB_Monoid_new_UDT(&BF_lMIN_Tuple3_Monoid, BF_lMIN_Tuple3, &BF_identity);

	//GrB_Semiring
	GrB_Semiring_new(&BF_lMIN_PLUSrhs_Tuple3, BF_lMIN_Tuple3_Monoid, BF_PLUSrhs_Tuple3);

	//--------------------------------------------------------------------------
	// allocate arrays used for tuplets
	//--------------------------------------------------------------------------
	I = rm_malloc(sizeof(GrB_Index) * nz) ;
	J = rm_malloc(sizeof(GrB_Index) * nz) ;
	w = rm_malloc(sizeof(double) * nz) ;
	W = rm_malloc(sizeof(BF_Tuple3_struct) * nz) ;
	if(I == NULL || J == NULL || w == NULL || W == NULL) {
		return GrB_OUT_OF_MEMORY;
	}

	//--------------------------------------------------------------------------
	// create matrix Atmp based on A, while its entries become BF_Tuple3 type
	//--------------------------------------------------------------------------
	GrB_Matrix_extractTuples_FP64(I, J, w, &nz, A);
	for(GrB_Index k = 0; k < nz; k++) {
		W[k] = (BF_Tuple3_struct) {
			.w = w[k], .h = 1, .pi = I[k] + 1
		};
	}
	GrB_Matrix_new(&Atmp, BF_Tuple3, n, n);
	GrB_Matrix_build_UDT(Atmp, I, J, W, nz, BF_lMIN_Tuple3);

	//--------------------------------------------------------------------------
	// create and initialize "distance" vector d, dmasked and dless
	//--------------------------------------------------------------------------
	GrB_Vector_new(&d, BF_Tuple3, n);
	// make d dense
	GrB_Vector_assign_UDT(d, NULL, NULL, (void *)&BF_identity, GrB_ALL, n, NULL);
	// initial distance from s to itself
	BF_Tuple3_struct d0 = (BF_Tuple3_struct) {
		.w = 0, .h = 0, .pi = 0
	};
	GrB_Vector_setElement_UDT(d, &d0, s);

	// creat dmasked as a sparse vector with only one entry at s
	GrB_Vector_new(&dmasked, BF_Tuple3, n);
	GrB_Vector_setElement_UDT(dmasked, &d0, s);

	// create dless
	GrB_Vector_new(&dless, GrB_BOOL, n);

	//--------------------------------------------------------------------------
	// start the Bellman Ford process
	//--------------------------------------------------------------------------
	bool any_dless = true;     // if there is any newly found shortest path
	int64_t iter = 0;          // number of iterations

	// terminate when no new path is found or more than V-1 loops
	while(any_dless && iter < n - 1) {
		// execute semiring on d and A, and save the result to dtmp
		GrB_vxm(dmasked, GrB_NULL, GrB_NULL, BF_lMIN_PLUSrhs_Tuple3, dmasked, Atmp, GrB_NULL);

		// dless = d .< dtmp
		GrB_Vector_clear(dless);
		GrB_eWiseMult(dless, NULL, NULL, BF_LT_Tuple3, dmasked, d, NULL);

		// if there is no entry with smaller distance then all shortest paths
		// are found
		GrB_reduce(&any_dless, NULL, GxB_LOR_BOOL_MONOID, dless, NULL) ;
		if(any_dless) {
			// update all entries with smaller distances
			GrB_apply(d, dless, NULL, BF_Identity_Tuple3, dmasked, NULL);

			// only use entries that were just updated
			GrB_Vector_clear(dmasked);
			GrB_apply(dmasked, dless, NULL, BF_Identity_Tuple3, d, NULL);
		}
		iter ++;
	}

	// check for negative-weight cycle only when there was a new path in the
	// last loop, otherwise, there can't be a negative-weight cycle.
	if(any_dless) {
		// execute semiring again to check for negative-weight cycle
		GrB_vxm(dmasked, GrB_NULL, GrB_NULL, BF_lMIN_PLUSrhs_Tuple3, dmasked, Atmp, GrB_NULL);

		// dless = d .< dtmp
		GrB_Vector_clear(dless);
		GrB_eWiseMult(dless, NULL, NULL, BF_LT_Tuple3, dmasked, d, NULL);

		// if there is no entry with smaller distance then all shortest paths
		// are found
		GrB_reduce(&any_dless, NULL, GxB_LOR_BOOL_MONOID, dless, NULL);
		if(any_dless) {
			// printf("A negative-weight cycle found. \n");
			FREE_ALL;
			return (GrB_SUCCESS) ;
		}
	}

	//--------------------------------------------------------------------------
	// extract tuple from "distance" vector d and create GrB_Vectors for output
	//--------------------------------------------------------------------------
	GrB_Vector_extractTuples_UDT(I, (void *) W, &n, d);
	h  = rm_malloc(sizeof(GrB_Index) * n) ;
	pi = rm_malloc(sizeof(GrB_Index) * n) ;
	if(w == NULL || h == NULL || pi == NULL) {
		return GrB_OUT_OF_MEMORY;
	}

	for(GrB_Index k = 0; k < n; k++) {
		w [k] = W[k].w ;
		h [k] = W[k].h ;
		pi[k] = W[k].pi;
	}
	GrB_Vector_new(pd_output,  GrB_FP64,   n);
	GrB_Vector_new(ppi_output, GrB_UINT64, n);
	GrB_Vector_new(ph_output,  GrB_UINT64, n);
	GrB_Vector_build_FP64(*pd_output, I, w, n, GrB_MIN_FP64);
	GrB_Vector_build_UINT64(*ppi_output, I, pi, n, GrB_MIN_UINT64);
	GrB_Vector_build_UINT64(*ph_output, I, h, n, GrB_MIN_UINT64);
	FREE_ALL;
	return (GrB_SUCCESS) ;
}
