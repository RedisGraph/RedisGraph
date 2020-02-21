//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum_template: C = A+B where all 3 matrices are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_unused.h"

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // any matrix may be aliased to any other (C==A, C==B, and/or A==B)
    GB_ATYPE *Ax = A->x ;
    GB_BTYPE *Bx = B->x ;
    GB_CTYPE *Cx = C->x ;
    const int64_t cnz = GB_NNZ (C) ;
    ASSERT (GB_is_dense (A)) ;
    ASSERT (GB_is_dense (B)) ;
    ASSERT (GB_is_dense (C)) ;
    int64_t p ;

    //--------------------------------------------------------------------------
    // C = A+B where all 3 matrices are dense
    //--------------------------------------------------------------------------

    if (C == B)
    {

        //----------------------------------------------------------------------
        // C = A+C where A and C are dense
        //----------------------------------------------------------------------

        #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Ax, Cx, nthreads) ;   // C += A

        #elif GB_HAS_CBLAS & GB_OP_IS_MINUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Ax, Cx, nthreads) ;  // C -= A

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;                  // aij = Ax [p]
                GB_BINOP (GB_CX (p), aij, GB_CX (p)) ;  // Cx [p] = aij + Cx [p]
            }

        #endif

    }
    else if (C == A)
    {

        //----------------------------------------------------------------------
        // C = C+B where B and C are dense
        //----------------------------------------------------------------------

        #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Bx, Cx, nthreads) ;   // C += B

        #elif GB_HAS_CBLAS & GB_OP_IS_MINUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Bx, Cx, nthreads) ;  // C -= B

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETB (bij, Bx, p) ;                  // bij = Bx [p]
                GB_BINOP (GB_CX (p), GB_CX (p), bij) ;  // Cx [p] += bij
            }

        #endif

    }
    else
    {

        //----------------------------------------------------------------------
        // C = A+B where all 3 matrices are dense
        //----------------------------------------------------------------------

        // note that A and B may still be aliased to each other

        #if GB_HAS_CBLAS && GB_OP_IS_PLUS_REAL

            GB_memcpy (Cx, Ax, cnz * sizeof (GB_CTYPE), nthreads) ; // C = A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Bx, Cx, nthreads) ;   // C += B

        #elif GB_HAS_CBLAS && GB_OP_IS_MINUS_REAL

            GB_memcpy (Cx, Ax, cnz * sizeof (GB_CTYPE), nthreads) ; // C = A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Bx, Cx, nthreads) ;  // C -= B

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;              // aij = Ax [p]
                GB_GETB (bij, Bx, p) ;              // bij = Bx [p]
                GB_BINOP (GB_CX (p), aij, bij) ;    // Cx [p] = aij + bij
            }

        #endif
    }
}

