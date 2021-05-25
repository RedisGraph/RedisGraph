//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum_template: C = A+B where all 3 matrices are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_unused.h"

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // any matrix may be aliased to any other (C==A, C==B, and/or A==B)
    GB_ATYPE *Ax = (GB_ATYPE *) A->x ;
    GB_BTYPE *Bx = (GB_BTYPE *) B->x ;
    GB_CTYPE *Cx = (GB_CTYPE *) C->x ;
    const int64_t cnz = GB_NNZ (C) ;
    ASSERT (GB_is_dense (A)) ;
    ASSERT (GB_is_dense (B)) ;
    ASSERT (GB_is_dense (C)) ;
    int64_t p ;

    //--------------------------------------------------------------------------
    // C = A+B where all 3 matrices are dense
    //--------------------------------------------------------------------------

    #if GB_CTYPE_IS_BTYPE

    if (C == B)
    {

        //----------------------------------------------------------------------
        // C = A+C where A and C are dense
        //----------------------------------------------------------------------

        // C and B cannot be aliased if their types differ

        #if defined ( GB_HAS_CBLAS ) && GB_OP_IS_PLUS_REAL

            // C += A via GB_cblas_saxpy or GB_cblas_daxpy
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Ax, Cx, nthreads) ;   // C += A

        #elif defined ( GB_HAS_CBLAS ) && GB_OP_IS_MINUS_REAL

            // C -= A via GB_cblas_saxpy or GB_cblas_daxpy
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Ax, Cx, nthreads) ;  // C -= A

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;                  // aij = Ax [p]
                // Cx [p] = aij + Cx [p]
                GB_BINOP (GB_CX (p), aij, GB_CX (p), 0, 0) ;
            }

        #endif

    }
    else 
    #endif

    #if GB_CTYPE_IS_ATYPE

    if (C == A)
    {

        //----------------------------------------------------------------------
        // C = C+B where B and C are dense
        //----------------------------------------------------------------------

        #if defined ( GB_HAS_CBLAS ) && GB_OP_IS_PLUS_REAL

            // C += B via GB_cblas_saxpy or GB_cblas_daxpy
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Bx, Cx, nthreads) ;   // C += B

        #elif defined ( GB_HAS_CBLAS ) && GB_OP_IS_MINUS_REAL

            // C -= B via GB_cblas_saxpy or GB_cblas_daxpy
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Bx, Cx, nthreads) ;  // C -= B

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETB (bij, Bx, p) ;                  // bij = Bx [p]
                GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ; // Cx [p] += bij
            }

        #endif

    }
    else
    #endif

    {

        //----------------------------------------------------------------------
        // C = A+B where all 3 matrices are dense
        //----------------------------------------------------------------------

        // note that A and B may still be aliased to each other

        #if defined ( GB_HAS_CBLAS ) && GB_OP_IS_PLUS_REAL

            // C = A+B via GB_cblas_saxpy or GB_cblas_daxpy
            GB_memcpy (Cx, Ax, cnz * sizeof (GB_CTYPE), nthreads) ; // C = A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Bx, Cx, nthreads) ;   // C += B

        #elif defined ( GB_HAS_CBLAS ) && GB_OP_IS_MINUS_REAL

            // C = A-B via GB_cblas_saxpy or GB_cblas_daxpy
            GB_memcpy (Cx, Ax, cnz * sizeof (GB_CTYPE), nthreads) ; // C = A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Bx, Cx, nthreads) ;  // C -= B

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;              // aij = Ax [p]
                GB_GETB (bij, Bx, p) ;              // bij = Bx [p]
                GB_BINOP (GB_CX (p), aij, bij, 0, 0) ;  // Cx [p] = aij + bij
            }

        #endif
    }
}

