//------------------------------------------------------------------------------
// GB_dense_ewise3_accum_template: C += A+B where all 3 matrices are dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// FUTURE: allow the accum and the 'plus' op to differ (as in C += A-B,
// with PLUS as the accum and MINUS as the operator, so CBLAS can be used
// for this combination.

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // any matrix may be aliased to any other (C==A, C==B, and/or A==B)
    GB_ATYPE *Ax = A->x ;
    GB_BTYPE *Bx = B->x ;
    GB_CTYPE *Cx = C->x ;
    const int64_t cnz = GB_NNZ (C) ;
    int64_t p ;

    //--------------------------------------------------------------------------
    // C += A+B where all 3 matries are dense
    //--------------------------------------------------------------------------

    if (A == B)
    {

        //----------------------------------------------------------------------
        // C += A+A where A and C are dense
        //----------------------------------------------------------------------

        // If the op is PLUS, this becomes C += 2*A.  If the op is MINUS,
        // almost nothing happens since C=C-(A-A) = C, except if A has Infs or
        // NaNs.  In this case, don't bother to call the CBLAS if the op is
        // MINUS.

        #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 2, Ax, Cx, nthreads) ;   // C += 2*A

        #else

            // C += A+A
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;                  // aij = Ax [p]
                GB_CTYPE_SCALAR (t) ;                   // declare scalar t
                GB_BINOP (t, aij, aij) ;                // t = aij + aij
                GB_BINOP (GB_CX (p), GB_CX (p), t) ;    // Cx [p] = cij + t
            }

        #endif

    }
    else
    {

        //----------------------------------------------------------------------
        // C += A+B where all 3 matrices are dense
        //----------------------------------------------------------------------

        #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Ax, Cx, nthreads) ;   // C += A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) 1, Bx, Cx, nthreads) ;   // C += B

        #elif GB_HAS_CBLAS & GB_OP_IS_MINUS_REAL

            // C -= (A-B)
            GB_CBLAS_AXPY (cnz, (GB_CTYPE) -1, Ax, Cx, nthreads) ;  // C -= A
            GB_CBLAS_AXPY (cnz, (GB_CTYPE)  1, Bx, Cx, nthreads) ;  // C += B

        #else

            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETA (aij, Ax, p) ;                  // aij = Ax [p]
                GB_GETB (bij, Bx, p) ;                  // bij = Bx [p]
                GB_CTYPE_SCALAR (t) ;                   // declare scalar t
                GB_BINOP (t, aij, bij) ;                // t = aij + bij
                GB_BINOP (GB_CX (p), GB_CX (p), t) ;    // Cx [p] = cij + t
            }

        #endif
    }
}

