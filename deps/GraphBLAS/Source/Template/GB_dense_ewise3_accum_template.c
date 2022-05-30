//------------------------------------------------------------------------------
// GB_dense_ewise3_accum_template: C += A+B where all 3 matrices are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// No matrix is iso.

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // any matrix may be aliased to any other (C==A, C==B, and/or A==B)
    GB_ATYPE *Ax = (GB_ATYPE *) A->x ;
    GB_BTYPE *Bx = (GB_BTYPE *) B->x ;
    GB_CTYPE *Cx = (GB_CTYPE *) C->x ;
    const int64_t cnz = GB_nnz (C) ;
    ASSERT (!C->iso) ;
    ASSERT (!A->iso) ;
    ASSERT (!B->iso) ;
    int64_t p ;

    //--------------------------------------------------------------------------
    // C += A+B where all 3 matries are dense
    //--------------------------------------------------------------------------

    if (A == B)
    {

        //----------------------------------------------------------------------
        // C += A+A where A and C are dense
        //----------------------------------------------------------------------

        // C += A+A
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETA (aij, Ax, p, false) ;           // aij = Ax [p]
            GB_CTYPE_SCALAR (t) ;                   // declare scalar t
            GB_BINOP (t, aij, aij, 0, 0) ;          // t = aij + aij
            GB_BINOP (GB_CX (p), GB_CX (p), t, 0, 0) ; // Cx [p] = cij + t
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C += A+B where all 3 matrices are dense
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETA (aij, Ax, p, false) ;           // aij = Ax [p]
            GB_GETB (bij, Bx, p, false) ;           // bij = Bx [p]
            GB_CTYPE_SCALAR (t) ;                   // declare scalar t
            GB_BINOP (t, aij, bij, 0, 0) ;          // t = aij + bij
            GB_BINOP (GB_CX (p), GB_CX (p), t, 0, 0) ; // Cx [p] = cij + t
        }
    }
}

