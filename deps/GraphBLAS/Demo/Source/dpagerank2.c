//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Demo/Source/dpagerank3: pagerank using a real semiring
//------------------------------------------------------------------------------

// PageRank via EXTREME GraphBLAS-ing!

// A is a square unsymmetric binary matrix of size n-by-n, where A(i,j) is the
// edge (i,j).  Self-edges are OK.  A can be of any built-in type.

// On output, P is pointer to an array of PageRank structs.  P[0] is the
// highest ranked page, with pagerank P[0].pagerank and the page corresponds to
// node number P[0].page in the graph.  P[1] is the next page, and so on, to
// the lowest-ranked page P[n-1].page with rank P[n-1].pagerank.

// This version operates on the original matrix A, without changing it.  The
// entire computation is done via a set of user-defined objects:  a type,
// several operators, a monoid, and a semiring.  If PAGERANK_PREEFINED is
// defined at compile time, then the GraphBLAS PageRank_* objects are assumed
// to be available as global objects.

// Acknowledgements:  this method was written with input from Richard Veras,
// Franz Franchetti, and Scott McMillan, Carnegie Mellon University.

//------------------------------------------------------------------------------
// helper macros
//------------------------------------------------------------------------------

// free all workspace
#define FREEWORK                    \
{                                   \
    GrB_free (&rdouble) ;           \
    GrB_free (&r) ;                 \
    GrB_free (&rnew) ;              \
    GrB_free (&dout) ;              \
    GrB_free (&rdiff) ;             \
    GrB_free (&desc) ;              \
    if (I != NULL) free (I) ;       \
    if (X != NULL) free (X) ;       \
    GrB_free (&PageRank_accum) ;    \
    GrB_free (&PageRank_add) ;      \
    GrB_free (&PageRank_monoid) ;   \
    GrB_free (&PageRank_multiply) ; \
    GrB_free (&PageRank_semiring) ; \
    GrB_free (&PageRank_diff) ;     \
    GrB_free (&PageRank_type) ;     \
    GrB_free (&PageRank_div) ;      \
    GrB_free (&PageRank_get) ;      \
    GrB_free (&PageRank_init) ;     \
}

// error handler: free output P and all workspace (used by CHECK and OK macros)
#define FREE_ALL                \
{                               \
    if (P != NULL) free (P) ;   \
    FREEWORK ;                  \
}

#include "demos.h"

//------------------------------------------------------------------------------
// scalar types and operators
//------------------------------------------------------------------------------

#ifndef PAGERANK_PREDEFINED

// each node has a rank value, and a constant which is 1/outdegree
typedef struct
{
    double rank ;
    double invdegree ;
}
pagerank_type ;

double pagerank_damping, pagerank_teleport, pagerank_rdiff,
    pagerank_init_rank, pagerank_rsum ;

// identity value for the pagerank_add monoid
pagerank_type pagerank_zero = { 0, 0 } ;

// unary operator to divide a double entry by the scalar pagerank_rsum
void pagerank_div (double *z, const double *x)
{
    (*z) = (*x) / pagerank_rsum ;
}

// unary operator that typecasts PageRank_type to double, extracting the rank
void pagerank_get_rank (double *z, const pagerank_type *x)
{
    (*z) = (x->rank) ;
}

// unary operator to initialize a node
void init_page (pagerank_type *z, const double *x)
{
    z->rank = pagerank_init_rank ;  // all nodes start with rank 1/n
    z->invdegree = 1. / (*x) ;      // set 1/outdegree of this node 
}

//------------------------------------------------------------------------------
// PageRank semiring
//------------------------------------------------------------------------------

// In MATLAB notation, the new rank is computed with:
// newrank = pagerank_damping * (rank * D * A) + pagerank_teleport

// where A is a square binary matrix of the original graph, and A(i,j)=1 if
// page i has a link to page j.  rank is a row vector of size n.  The matrix D
// is diagonal, with D(i,i)=1/outdegree(i), where outdegree(i) = the outdegree
// of node i, or equivalently, outdegree(i) = sum (A (i,:)).

// That is, if newrank(j) were computed with a dot product:
//      newrank (j) = 0
//      for all i:
//          newrank (j) = newrank (j) + (rank (i) * D (i,i)) * A (i,j)

// To accomplish this computation in a single vector-matrix multiply, the value
// of D(i,i) is held as component of a combined data type, the pagerank_type,
// which has both the rank(i) and the entry D(i,i).

// binary multiplicative operator for the pagerank semiring
void pagerank_multiply
(
    pagerank_type *z,
    const pagerank_type *x,
    const bool *y
)
{
    // y is the boolean entry of the matrix, A(i,j)
    // x->rank is the rank of node i, and x->invdegree is 1/outdegree(i)
    // note that z->invdegree is left unchanged
    z->rank = (*y) ? ((x->rank) * (x->invdegree)) : 0 ;
}

// binary additive operator for the pagerank semiring
void pagerank_add
(
    pagerank_type *z,
    const pagerank_type *x,
    const pagerank_type *y
)
{
    // note that z->invdegree is left unchanged; it is unused
    z->rank = (x->rank) + (y->rank) ;
}

//------------------------------------------------------------------------------
// pagerank accumulator
//------------------------------------------------------------------------------

// The semiring computes the vector newrank = rank*D*A.  To complete the page
// rank computation, the new rank must be scaled by the
// pagerank_damping, and the pagerank_teleport must be included, which is
// done in the page rank accumulator:

// newrank = pagerank_damping * newrank + pagerank_teleport

// The PageRank_semiring does not construct the entire pagerank_type of
// rank*D*A, since the vector that holds newrank(i) must also keep the
// 1/invdegree(i), unchanged.  This is restored in the accumulator operator.

// The PageRank_accum operator can also compute pagerank_rdiff = norm (r-rnew),
// as a side effect.  This is unsafe but faster (see the comments below);
// uncomment the following #define to enable the unsafe method, or comment it
// out to use the safe method:
//
//  #define PAGERANK_UNSAFE

// binary operator to accumulate the new rank from the old
void pagerank_accum
(
    pagerank_type *z,
    const pagerank_type *x,
    const pagerank_type *y
)
{
    // note that this formula does not use the old rank:
    // new rank = pagerank_damping * (rank*A ) + pagerank_teleport
    double rnew = pagerank_damping * (y->rank) + pagerank_teleport ;

    #ifdef PAGERANK_UNSAFE

    // This computation of pagerank_rdiff is not guaranteed to work per the
    // GraphBLAS spec, but it does work with the current implementation of
    // SuiteSparse:GraphBLAS.  The reason is that there is no guarantee that
    // the accumulator step of a GraphBLAS operation is computed sequentially.
    // If computed in parallel, a race condition would occur.

    // This step uses the old rank, to compute the stopping criterion:
    // pagerank_rdiff = sum (ranknew - rankold)
    double delta = rnew - (x->rank) ;
    pagerank_rdiff += delta * delta ;

    #endif

    // update the rank, and copy over the inverse degree from the old page info
    z->rank = rnew ;
    z->invdegree = x->invdegree ;
}

//------------------------------------------------------------------------------
// pagerank_diff: compute the change in the rank
//------------------------------------------------------------------------------

// This is safer than computing pagerank_rdiff via pagerank_accum, and is
// compliant with the GraphBLAS spec.

void pagerank_diff
(
    pagerank_type *z,
    const pagerank_type *x,
    const pagerank_type *y
)
{
    double delta = (x->rank) - (y->rank) ;
    z->rank = delta * delta ;
}

#endif

//------------------------------------------------------------------------------
// comparison function for qsort
//------------------------------------------------------------------------------

int pagerank_compar (const void *x, const void *y)
{
    PageRank *a = (PageRank *) x ;
    PageRank *b = (PageRank *) y ;

    // sort by pagerank in descending order
    if (a->pagerank > b->pagerank)
    {
        return (-1) ;
    }
    else if (a->pagerank == b->pagerank)
    {
        return (0) ;
    }
    else
    {
        return (1) ;
    }
}

//------------------------------------------------------------------------------
// dpagerank2: compute the PageRank of all nodes in a graph
//------------------------------------------------------------------------------

GrB_Info dpagerank2         // GrB_SUCCESS or error condition
(
    PageRank **Phandle,     // output: pointer to array of PageRank structs
    GrB_Matrix A,           // input graph, not modified
    int itermax,            // max number of iterations
    double tol,             // stop when norm (r-rnew,2) < tol
    int *iters,             // number of iterations taken
    GrB_Desc_Value method   // method to use for GrB_vxm (for testing only)
)
{

    GrB_Info info ;
    double *X = NULL ;
    GrB_Index n, *I = NULL ;
    PageRank *P = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Vector r = NULL, dout = NULL, rdouble = NULL, rnew = NULL, rdiff = NULL;

    //--------------------------------------------------------------------------
    // create the new type, operators, monoid, and semiring
    //--------------------------------------------------------------------------

    #ifndef PAGERANK_PREDEFINED

    // PageRank_* objects are not defined at compile time (my_pagerank.m4 is
    // not in the User/ directory).  Define them here at run-time:

    GrB_Type PageRank_type = NULL ;
    GrB_UnaryOp PageRank_div = NULL, PageRank_get = NULL, PageRank_init = NULL ;
    GrB_BinaryOp PageRank_accum = NULL, PageRank_add = NULL,
        PageRank_multiply = NULL, PageRank_diff = NULL ;
    GrB_Monoid PageRank_monoid = NULL ;
    GrB_Semiring PageRank_semiring = NULL ;

    // create the new Page type
    OK (GrB_Type_new (&PageRank_type, sizeof (pagerank_type))) ;

    // create the unary operator to initialize the PageRank_type of each node
    OK (GrB_UnaryOp_new (&PageRank_init, init_page, PageRank_type, GrB_FP64)) ;

    // create PageRank_accum
    OK (GrB_BinaryOp_new (&PageRank_accum, pagerank_accum,
        PageRank_type, PageRank_type, PageRank_type)) ;

    // create PageRank_add operator and monoid
    OK (GrB_BinaryOp_new (&PageRank_add, pagerank_add,
        PageRank_type, PageRank_type, PageRank_type)) ;
    OK (GrB_Monoid_new_UDT (&PageRank_monoid, PageRank_add, &pagerank_zero)) ;

    // create PageRank_multiply operator
    OK (GrB_BinaryOp_new (&PageRank_multiply, pagerank_multiply,
        PageRank_type, PageRank_type, GrB_BOOL)) ;

    // create PageRank_semiring
    OK (GrB_Semiring_new (&PageRank_semiring, PageRank_monoid,
        PageRank_multiply)) ;

    // create unary operator that typecasts the PageRank_type to double
    OK (GrB_UnaryOp_new (&PageRank_get, pagerank_get_rank, GrB_FP64,
        PageRank_type)) ;

    // create unary operator that scales the rank by pagerank_rsum
    OK (GrB_UnaryOp_new (&PageRank_div, pagerank_div, GrB_FP64, GrB_FP64)) ;

    // create PageRank_diff operator
    OK (GrB_BinaryOp_new (&PageRank_diff, pagerank_diff,
        PageRank_type, PageRank_type, PageRank_type)) ;

    printf ("dpagerank2: pagerank objects defined at run-time\n") ;
    #else
    printf ("dpagerank2: pagerank objects defined at compile-time\n") ;
    #endif

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    (*Phandle) = NULL ;

    // n = size (A,1) ;         // number of nodes
    OK (GrB_Matrix_nrows (&n, A)) ;

    // probability of walking to random neighbor
    pagerank_damping = 0.85 ;

    // dout = sum (A,2) ;       // dout(i) is the out-degree of node i
    OK (GrB_Vector_new (&dout, GrB_FP64, n)) ;
    OK (GrB_reduce (dout, NULL, NULL, GrB_PLUS_FP64, A, NULL)) ;

    // all nodes start with rank 1/n
    pagerank_init_rank = 1.0 / ((double) n) ;

    // initialize the page rank and inverse degree of each node
    OK (GrB_Vector_new (&r, PageRank_type, n)) ;
    OK (GrB_apply (r, NULL, NULL, PageRank_init, dout, NULL)) ;

    // dout vector no longer needed
    OK (GrB_free (&dout)) ;

    // to jump to any random node in entire graph:
    pagerank_teleport = (1-pagerank_damping) / n ;

    tol = tol*tol ;             // use tol^2 so sqrt(pagerank_rdiff) not needed
    pagerank_rdiff = 1 ;        // so first iteration is always done

    // create rdouble, a double vector of size n
    OK (GrB_Vector_new (&rdouble, GrB_FP64, n)) ;

    // GxB_print (A, GxB_SUMMARY) ;

    #ifndef PAGERANK_UNSAFE
    // the safe version requires another vector, rnew.  Note that dup is
    // needed, since the invdegree is copied by the PageRank_accum.
    OK (GrB_Vector_dup (&rnew, r)) ;
    OK (GrB_Vector_new (&rdiff, PageRank_type, n)) ;
    #endif

    // select method for GrB_vxm (for testing only; default is fine)
    if (method != GxB_DEFAULT)
    {
        OK (GrB_Descriptor_new (&desc)) ;
        OK (GxB_set (desc, GxB_AxB_METHOD, method)) ;
    }

    //--------------------------------------------------------------------------
    // iterate to compute the pagerank of each node
    //--------------------------------------------------------------------------

    for ((*iters) = 0 ; (*iters) < itermax && pagerank_rdiff > tol ; (*iters)++)
    {

        #ifdef PAGERANK_UNSAFE

            //------------------------------------------------------------------
            // pagerank: dangerous version (works but is not guaranteed to work)
            //------------------------------------------------------------------

            // r = pagerank_damping * (r * D * A) + pagerank_teleport,
            // and compute pagerank_rdiff = sum ((r - rnew).^2)
            pagerank_rdiff = 0 ;
            OK (GrB_vxm (r, NULL, PageRank_accum, PageRank_semiring, r, A,
                desc)) ;

            // SuiteSparse:GraphBLAS does not require this, since there will be
            // no pending operations from GrB_vxm above in the current version>
            // However, the PageRank_accum operator computes pagerank_rdiff as
            // a side effect, and it is needed to terminate the for loop.  So
            // we must ensure the computations have been done, in case the
            // GrB_vxm, or part of it, remains pending.
            OK (GrB_wait ( )) ;

            // The following would be better, since it would just finish the
            // computations for r, not all other computations:
            // GrB_Index ignore ;
            // OK (GrB_Vector_nvals (&ignore, r)) ;

            // Waiting for GrB_vxm to compute is necessary but not sufficient;
            // the above computation is still unsafe.  It will fail if
            // SuiteSparse:GraphBLAS computes the accumulator step in parallel.

        #else

            //------------------------------------------------------------------
            // pagerank: safe version
            //------------------------------------------------------------------

            // rnew = pagerank_damping * (r * D * A) + pagerank_teleport
            OK (GrB_vxm (rnew, NULL, PageRank_accum, PageRank_semiring, r, A,
                desc)) ;

            // compute pagerank_rdiff = sum ((r - rnew).^2)
            OK (GrB_eWiseAdd (rdiff, NULL, NULL, PageRank_diff, r, rnew, NULL));
            pagerank_type rsum ;
            OK (GrB_reduce (&rsum, NULL, PageRank_monoid, rdiff, NULL)) ;
            pagerank_rdiff = rsum.rank ;

            // r = rnew, using a swap, which is faster than GrB_assign or dup
            GrB_Vector rtemp = r ;
            r = rnew ;
            rnew = rtemp ;

        #endif
    }

    //--------------------------------------------------------------------------
    // scale the result: rdouble = rank / sum(r)
    //--------------------------------------------------------------------------

    // rnew (for the safe version) is no longer needed
    GrB_free (&rnew) ;

    // rdouble = pagerank_get_rank (r)
    OK (GrB_apply (rdouble, NULL, NULL, PageRank_get, r, NULL)) ;

    // r no longer needed
    GrB_free (&r) ;

    // pagerank_rsum = sum (rdouble)
    OK (GrB_reduce (&pagerank_rsum, NULL, GxB_PLUS_FP64_MONOID, rdouble, NULL));

    // r = r / pagerank_rsum
    OK (GrB_apply (rdouble, NULL, NULL, PageRank_div, rdouble, NULL)) ;

    //--------------------------------------------------------------------------
    // sort the nodes by pagerank
    //--------------------------------------------------------------------------

    // GraphBLAS does not have a mechanism to sort the components of a vector,
    // so it must be done by extracting and then sorting the tuples from
    // the GrB_Vector rdouble.

    // [r,irank] = sort (r, 'descend') ;

    // [I,X] = find (r) ;
    X = malloc (n * sizeof (double)) ;
    I = malloc (n * sizeof (GrB_Index)) ;
    CHECK (I != NULL && X != NULL, GrB_OUT_OF_MEMORY) ;
    GrB_Index nvals = n ;
    OK (GrB_Vector_extractTuples (I, X, &nvals, rdouble)) ;

    // rdouble no longer needed
    GrB_free (&rdouble) ;

    // P = struct (X,I)
    P = malloc (n * sizeof (PageRank)) ;
    CHECK (P != NULL, GrB_OUT_OF_MEMORY) ;
    int64_t k ;
    for (k = 0 ; k < nvals ; k++)
    {
        // The kth ranked page is P[k].page (with k=0 being the highest rank),
        // and its pagerank is P[k].pagerank.
        P [k].pagerank = X [k] ;
        // I [k] == k will be true for SuiteSparse:GraphBLAS but in general I
        // can be returned in any order, so use I [k] instead of k, for other
        // GraphBLAS implementations.
        P [k].page = I [k] ;
    }
    for ( ; k < n ; k++)
    {
        // If A has empty columns, then r will become sparse.  In this case,
        // pages with no incoming edges will be unranked.  The drowscale
        // function avoids this problem by adding a
        P [k].pagerank = 0 ;
        P [k].page = -1 ;
    }

    // free workspace
    FREEWORK ;

    // qsort (P) in descending order
    qsort (P, nvals, sizeof (PageRank), pagerank_compar) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*Phandle) = P ;
    return (GrB_SUCCESS) ;
}

