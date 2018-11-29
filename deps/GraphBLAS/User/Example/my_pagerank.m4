//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_pagerank.m4: PageRank semiring
//------------------------------------------------------------------------------

// Defines a PageRank type, operators, monoid, and semiring for the method in
// Demo/Source/dpagerank2.c.

#ifdef GxB_USER_INCLUDE

// Define a token that dpagerank2.c can use to determine if these definitions
// are available at compile-time.
#define PAGERANK_PREDEFINED

// each node has a rank value, and a constant which is 1/outdegree
typedef struct
{
    double rank ;
    double invdegree ;
}
pagerank_type ;

// global variable declarations
extern
double pagerank_damping, pagerank_teleport, pagerank_rdiff,
    pagerank_init_rank, pagerank_rsum ;

// for thread safety if the user application uses OpenMP, with parallel calls
// to dpagerank2 on independent problems.
#pragma omp threadprivate(pagerank_damping, pagerank_teleport, pagerank_rdiff, pagerank_init_rank, pagerank_rsum)

// The identity value for the pagerank_add monoid is {0,0}. For the
// GxB_*_define macro that defines the GrB_Monoid, the identity argument must
// be a compile-time constant (for the C definition), and it must also be
// parsable as an argument to the m4 macro.  If the user-defined type is a
// struct, the initializer uses curly brackets, but this causes a parsing error
// for m4.  The solution is to define a C macro with the initialization
// constant, and to use it in the GxB*define m4 macro.
#define PAGERANK_ZERO {0,0}

// unary operator to divide a double entry by the scalar pagerank_rsum
static inline
void pagerank_div (double *z, const double *x)
{
    (*z) = (*x) / pagerank_rsum ;
}

// unary operator that typecasts PageRank_type to double, extracting the rank
static inline
void pagerank_get_rank (double *z, const pagerank_type *x)
{
    (*z) = (x->rank) ;
}

// unary operator to initialize a node
static inline
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
static inline
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
static inline
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
    #define PAGERANK_UNSAFE

// binary operator to accumulate the new rank from the old
static inline
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

static inline
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

#else

// global variable definitions
double pagerank_damping, pagerank_teleport, pagerank_rdiff,
    pagerank_init_rank, pagerank_rsum ;

#endif

// create the new Page type
GxB_Type_define(PageRank_type, pagerank_type) ;

// create the unary operator to initialize the PageRank_type of each node
GxB_UnaryOp_define(PageRank_init, init_page, PageRank_type, GrB_FP64) ;

// create PageRank_accum
GxB_BinaryOp_define(PageRank_accum, pagerank_accum,
    PageRank_type, PageRank_type, PageRank_type) ;

// create PageRank_add operator and monoid
GxB_BinaryOp_define(PageRank_add, pagerank_add,
    PageRank_type, PageRank_type, PageRank_type) ;

// create PageRank_monoid.  See the discussion above for PAGERANK_ZERO.
GxB_Monoid_define(PageRank_monoid, PageRank_add, PAGERANK_ZERO) ;

// create PageRank_multiply operator
GxB_BinaryOp_define(PageRank_multiply, pagerank_multiply,
    PageRank_type, PageRank_type, GrB_BOOL) ;

// create PageRank_semiring
GxB_Semiring_define(PageRank_semiring, PageRank_monoid,
    PageRank_multiply) ;

// create unary operator that typecasts the PageRank_type to double
GxB_UnaryOp_define(PageRank_get, pagerank_get_rank, GrB_FP64,
    PageRank_type) ;

// create unary operator that scales the rank by pagerank_rsum
GxB_UnaryOp_define(PageRank_div, pagerank_div, GrB_FP64, GrB_FP64) ;

// create PageRank_diff operator
GxB_BinaryOp_define(PageRank_diff, pagerank_diff,
        PageRank_type, PageRank_type, PageRank_type) ;

