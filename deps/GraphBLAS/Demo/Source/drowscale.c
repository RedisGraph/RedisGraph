//------------------------------------------------------------------------------
// drowscale: scale the rows of an adjacency matrix by out-degree
//------------------------------------------------------------------------------

// on input, A is a square unsymmetric binary matrix of size n-by-n, of any
// built-in type.  On output, C is a rowscaled version of A, of type
// GrB_FP64, with C = D*A + I.  The diagonal matrix D has diagonal entries
// D(i,i)=1/sum(A(i,:)), or D(i,i) is not present if A(i,:) is empty.
// The matrix I has entries only on the diagonal, all equal to zero.  This
// optional step ensures that C has no empty columns, which speeds up the
// subsequent PageRank computation.

/* MATLAB equivalent (excluding the addition of I):

    function C = rowscale (A)
    %ROWSCALE row scale an adjacency matrix by out-degree
    % C = rowscale (A)

    % scale the adjacency matrix by out-degree
    dout = sum (A,2) ;              % dout(i) is the out-degree of node i
    is_nonempty = (dout > 0) ;      % find vertices with outgoing edges
    nonempty = find (is_nonempty) ; % list of vertices with outgoing edges
    n = size (A,1) ;

    % divide each non-empty row by its out-degree
    dinv = 1 ./ dout (is_nonempty) ;
    D = sparse (nonempty, nonempty, dinv, n, n) ;
    C = D*A ;
*/

//------------------------------------------------------------------------------
// helper macros
//------------------------------------------------------------------------------

// free all workspace
#define FREEWORK                \
{                               \
    GrB_free (&dout) ;          \
    GrB_free (&D) ;             \
    if (I != NULL) free (I) ;   \
    if (X != NULL) free (X) ;   \
}

// error handler: free workspace and the output matrix C
#define FREE_ALL                \
{                               \
    GrB_free (&C) ;             \
    FREEWORK ;                  \
}

#include "demos.h"

//------------------------------------------------------------------------------
// drowscale: C = D*A + I*0 where D(i,i) = 1/sum(A(i,:)
//------------------------------------------------------------------------------

GrB_Info drowscale          // GrB_SUCCESS or error condition
(
    GrB_Matrix *Chandle,    // output matrix C = rowscale (A)
    GrB_Matrix A            // input matrix, not modified
)
{

    //--------------------------------------------------------------------------
    // intializations
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Vector dout = NULL ;
    GrB_Matrix D = NULL, C = NULL ;
    GrB_Index *I = NULL ;
    double    *X = NULL ;

    // n = size (A,1) ;
    GrB_Index n ;
    OK (GrB_Matrix_nrows (&n, A)) ;

    //--------------------------------------------------------------------------
    // dout = sum (A,2) ;           // dout(i) is the out-degree of node i
    //--------------------------------------------------------------------------

    OK (GrB_Vector_new (&dout, GrB_FP64, n)) ;
    OK (GrB_reduce (dout, NULL, NULL, GrB_PLUS_FP64, A, NULL)) ;

    //--------------------------------------------------------------------------
    // construct scaling matrix D
    //--------------------------------------------------------------------------

    // D is diagonal with D(i,i) = 1/sum(A(i,:)), or D(i,i) is not present if
    // row i of A has no entries.

    // [I,~,X] = find (dout) ;
    I = malloc ((n+1) * sizeof (GrB_Index)) ;
    X = malloc ((n+1) * sizeof (double)) ;
    CHECK (I != NULL && X != NULL, GrB_OUT_OF_MEMORY) ;
    GrB_Index nvals = n ;
    OK (GrB_Vector_extractTuples (I, X, &nvals, dout)) ;

    // I and X exclude empty columns of A.  This condition is always true.
    CHECK (nvals <= n, GrB_PANIC) ;

    // D = diag (1./dout) ;
    OK (GrB_Matrix_new (&D, GrB_FP64, n, n)) ;
    for (int64_t i = 0 ; i < nvals ; i++)
    {
        X [i] = 1. / X [i] ;
    }
    OK (GrB_Matrix_build (D, I, I, X, nvals, GrB_PLUS_FP64)) ;

    //--------------------------------------------------------------------------
    // C = diagonal matrix with explicit zeros on diagonal
    //--------------------------------------------------------------------------

    // This step is optional, but it ensures that C has no non-empty columns,
    // which speeds up the pagerank computation by ensuring r*C remains a dense
    // vector.
    for (int64_t i = 0 ; i < n ; i++)
    {
        I [i] = i ;
        X [i] = 0 ;
    }
    OK (GrB_Matrix_new (&C, GrB_FP64, n, n)) ;
    OK (GrB_Matrix_build (C, I, I, X, n, GrB_PLUS_FP64)) ;

    //--------------------------------------------------------------------------
    // C += D*A
    //--------------------------------------------------------------------------

    OK (GrB_mxm (C, NULL, GrB_PLUS_FP64, GxB_PLUS_TIMES_FP64, D, A, NULL)) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    FREEWORK ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

