//------------------------------------------------------------------------------
// GB_dense_ewise3_noaccum_template: C = A+B where all 3 matrices are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
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
    const int64_t cnz = GB_nnz (C) ;
    ASSERT (GB_as_if_full (A)) ;
    ASSERT (GB_as_if_full (B)) ;
    ASSERT (GB_IS_FULL (C)) ;
    ASSERT (!C->iso) ;
    ASSERT (!A->iso) ;
    ASSERT (!B->iso) ;
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
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETA (aij, Ax, p, false) ;                // aij = Ax [p]
            GB_BINOP (GB_CX (p), aij, GB_CX (p), 0, 0) ; // Cx [p] = aij+Cx [p]
        }

    }
    else 
    #endif

    #if GB_CTYPE_IS_ATYPE

    if (C == A)
    {

        //----------------------------------------------------------------------
        // C = C+B where B and C are dense
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETB (bij, Bx, p, false) ;                   // bij = Bx [p]
            GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ;    // Cx [p] += bij
        }

    }
    else
    #endif

    {

        //----------------------------------------------------------------------
        // C = A+B where all 3 matrices are dense
        //----------------------------------------------------------------------

        // note that A and B may still be aliased to each other
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETA (aij, Ax, p, false) ;               // aij = Ax [p]
            GB_GETB (bij, Bx, p, false) ;               // bij = Bx [p]
            GB_BINOP (GB_CX (p), aij, bij, 0, 0) ;      // Cx [p] = aij + bij
        }
    }
}

