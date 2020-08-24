//------------------------------------------------------------------------------
// LAGraph_random: create a random matrix
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

// LAGraph_random: create a random matrix, contributed by Tim Davis, Texas A&M

// Creates a random matrix A of the given type, and dimension nrows-by-ncols.

// nvals:  roughly the number of entries to create.  If the matrix is made to
// be symmetric, skew-symmetric, or Hermitian, then this is the number of
// entries created in the lower triangular part (including the diagonal), so
// there are about twice as many entries in A as the given nvals.  If
// no_diagonal is true, entries on the diagonal are dropped, thus reducing the
// entries in A from the given nvals.

// The parameters are considered in the following order:

// type: A is always returned with this type.  If the type is not
//                  LAGraph_ComplexFP64, then make_hermitian is ignored and treated
//                  as if false.  If the type is unsigned, then
//                  make_skew_symmetric is ignored and treated as if false.

// if (nrows != ncols) then make_symmetric, make_skew_symmetric, and
//                  make_hermitian are ignored and treated as if false.

// make_pattern:  entries that appear in the matrix all have the value 1.
//                  If true, then make_skew_symmetric and make_hermitian
//                  are ignored and treated as if false.

// make_symmetric:  if true, then A will be symmetric.  The parameters
//                  make_skew_symmetric and make_hermitian are ignored and
//                  treated as if false.

// make_skew_symmetric: If true then A is skew-symmetric (A == -A.', where A.'
//                  denotes the array transpose).  no_diagonal and
//                  make_hermitian are ignored and treated as if true and
//                  false, respectively.

// make_hermitian:  if true, then A is Hermitian (A == -A', where A' denotes
//                  the complex conjugate transpose).  If type is not
//                  LAGraph_ComplexFP64, then make_hermitian is ignored and treated
//                  as if false.

// no_diagonal:  if true, then A is returned with no entries on the diagonal.

// seed: random number seed for LAGraph_rand.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
    GrB_free (A) ;

GrB_Info LAGraph_random         // create a random matrix
(
    GrB_Matrix *A,              // handle of matrix to create
    GrB_Type type,              // built-in type, or LAGraph_ComplexFP64
    GrB_Index nrows,            // number of rows
    GrB_Index ncols,            // number of columns
    GrB_Index nvals,            // number of values
    bool make_pattern,          // if true, A is a pattern
    bool make_symmetric,        // if true, A is symmetric
    bool make_skew_symmetric,   // if true, A is skew-symmetric
    bool make_hermitian,        // if trur, A is hermitian
    bool no_diagonal,           // if true, A has no entries on the diagonal
    uint64_t *seed              // random number seed; modified on return
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    if (A == NULL)
    {
        return (GrB_NULL_POINTER) ;
    }
    *A = NULL ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    if (type == GrB_BOOL || type == GrB_UINT8 || type == GrB_UINT16 ||
        type == GrB_UINT32 || type == GrB_UINT64)
    {
        make_skew_symmetric = false ;
    }

    if (nrows == 0 || ncols == 0)
    {
        nvals = 0 ;
    }

    if (nrows != ncols)
    {
        make_symmetric = false ;
        make_skew_symmetric = false ;
        make_hermitian = false ;
    }

    if (make_pattern || make_symmetric)
    {
        make_skew_symmetric = false ;
        make_hermitian = false ;
    }

    if (make_skew_symmetric)
    {
        make_hermitian = false ;
        no_diagonal = true ;
    }

    if (type != LAGraph_ComplexFP64)
    {
        make_hermitian = false ;
    }

    //--------------------------------------------------------------------------
    // construct the matrix
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_Matrix_new (A, type, nrows, ncols)) ;

    #define RX(ctype,gt,is_bool,is_int,is_signed,is_real,is_complex)           \
    {                                                                          \
        ctype x = 1 ;                                                          \
        for (int64_t k = 0 ; k < nvals ; k++)                                  \
        {                                                                      \
            /* get random row and column indices */                            \
            GrB_Index i = LAGraph_rand64 (seed) % nrows ;                      \
            GrB_Index j = LAGraph_rand64 (seed) % ncols ;                      \
            if (no_diagonal && (i == j)) continue ;                            \
            /* get a random value of the given type */                         \
            if (make_pattern)                                                  \
            {                                                                  \
                /* x is one */ ;                                               \
            }                                                                  \
            else if (is_bool)                                                  \
            {                                                                  \
                x = LAGraph_rand64 (seed) % 2 ;                                \
            }                                                                  \
            else if (is_int)                                                   \
            {                                                                  \
                uint64_t t = LAGraph_rand64 (seed) ;                           \
                memcpy (&x, &t, sizeof (ctype)) ;                              \
            }                                                                  \
            else if (is_real)                                                  \
            {                                                                  \
                x = (ctype) LAGraph_randx (seed) ;                             \
            }                                                                  \
            else if (is_complex)                                               \
            {                                                                  \
                double xreal = LAGraph_randx (seed) ;                          \
                double ximag = LAGraph_randx (seed) ;                          \
                x = CMPLX (xreal, ximag) ;                                     \
            }                                                                  \
            /* A (i,j) = x */                                                  \
            LAGRAPH_OK (GrB_Matrix_setElement_ ## gt (*A, ARG(x), i, j)) ;     \
            if (make_symmetric)                                                \
            {                                                                  \
                /* A (j,i) = x */                                              \
                LAGRAPH_OK (GrB_Matrix_setElement_ ## gt (*A, ARG(x), j, i)) ; \
            }                                                                  \
            else if (is_signed && make_skew_symmetric)                         \
            {                                                                  \
                /* A (j,i) = -x */                                             \
                x = -x ;                                                       \
                LAGRAPH_OK (GrB_Matrix_setElement_ ## gt (*A, ARG(x), j, i)) ; \
            }                                                                  \
            else if (is_complex && make_hermitian)                             \
            {                                                                  \
                /* A (j,i) = conj (x) */                                       \
                x = CONJ (x) ;                                                 \
                LAGRAPH_OK (GrB_Matrix_setElement_ ## gt (*A, ARG(x), j, i)) ; \
            }                                                                  \
        }                                                                      \
    }

    #define ARG(x) x
    #define CONJ(x) x
    uint64_t t [2] = {0,0} ;

    if      (type == GrB_BOOL        ) RX (bool    , BOOL  , 1, 0, 0, 0, 0)
    else if (type == GrB_INT8        ) RX (int8_t  , INT8  , 0, 1, 1, 0, 0)
    else if (type == GrB_INT16       ) RX (int16_t , INT16 , 0, 1, 1, 0, 0)
    else if (type == GrB_INT32       ) RX (int32_t , INT32 , 0, 1, 1, 0, 0)
    else if (type == GrB_INT64       ) RX (int64_t , INT64 , 0, 1, 1, 0, 0)
    else if (type == GrB_UINT8       ) RX (uint16_t, UINT8 , 0, 1, 0, 0, 0)
    else if (type == GrB_UINT16      ) RX (uint32_t, UINT16, 0, 1, 0, 0, 0)
    else if (type == GrB_UINT32      ) RX (uint64_t, UINT32, 0, 1, 0, 0, 0)
    else if (type == GrB_UINT64      ) RX (uint64_t, UINT64, 0, 1, 0, 0, 0)
    else if (type == GrB_FP32        ) RX (float   , FP32  , 0, 0, 1, 1, 0)
    else if (type == GrB_FP64        ) RX (double  , FP64  , 0, 0, 1, 1, 0)
    else if (type == LAGraph_ComplexFP64 )
    {
        #undef ARG
        #undef CONJ
        #define ARG(x) &(x)
        #define CONJ(x) conj(x)
        RX (double complex, UDT, 0, 0, 1, 0, 1) ;
    }
    else
    {
        // type not supported
        LAGRAPH_FREE_ALL ;
        return (GrB_INVALID_VALUE) ;
    }

    return (GrB_SUCCESS) ;
}

