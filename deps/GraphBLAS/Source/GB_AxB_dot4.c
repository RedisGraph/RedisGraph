//------------------------------------------------------------------------------
// GB_AxB_dot4: compute C+=A'*B in place
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_AxB_dot4 does its computation in a single phase, computing its result in
// the input matrix C, which is already dense.  The mask M is not handled by
// this function.

#include "GB_mxm.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"
#endif

#define GB_FREE_WORK                                            \
{                                                               \
    GB_FREE_MEMORY (A_slice, naslice+1, sizeof (int64_t)) ;     \
    GB_FREE_MEMORY (B_slice, nbslice+1, sizeof (int64_t)) ;     \
}

GrB_Info GB_AxB_dot4                // C+=A'*B, dot product method
(
    GrB_Matrix C,                   // input/output matrix, must be dense
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C+=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for dot in place += A'*B", GB0) ;
    ASSERT_MATRIX_OK (A, "A for dot in place += A'*B", GB0) ;
    ASSERT_MATRIX_OK (B, "B for dot in place += A'*B", GB0) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (GB_is_dense (C)) ;
    ASSERT_SEMIRING_OK (semiring, "semiring for in place += A'*B", GB0) ;
    ASSERT (A->vlen == B->vlen) ;

    int64_t *GB_RESTRICT A_slice = NULL ;
    int64_t *GB_RESTRICT B_slice = NULL ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    ASSERT (C->type     == add->op->ztype) ;

    bool op_is_first  = mult->opcode == GB_FIRST_opcode ;
    bool op_is_second = mult->opcode == GB_SECOND_opcode ;
    bool op_is_pair   = mult->opcode == GB_PAIR_opcode ;
    bool A_is_pattern = false ;
    bool B_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        A_is_pattern = op_is_first  || op_is_pair ;
        B_is_pattern = op_is_second || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        A_is_pattern = op_is_second || op_is_pair ;
        B_is_pattern = op_is_first  || op_is_pair ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->ytype))) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    int64_t bnz = GB_NNZ (B) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + bnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // slice A and B
    //--------------------------------------------------------------------------

    int64_t anvec = A->nvec ;
    int64_t bnvec = B->nvec ;

    int naslice = (nthreads == 1) ? 1 : (16 * nthreads) ;
    int nbslice = (nthreads == 1) ? 1 : (16 * nthreads) ;

    naslice = GB_IMIN (naslice, anvec) ;
    nbslice = GB_IMIN (nbslice, bnvec) ;

    if (!GB_pslice (&A_slice, A->p, anvec, naslice)  ||
        !GB_pslice (&B_slice, B->p, bnvec, nbslice))
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // C += A'*B, computing each entry with a dot product, via builtin semiring
    //--------------------------------------------------------------------------

    bool done = false ;

#ifndef GBCOMPACT

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Adot4B(add,mult,xyname) GB_Adot4B_ ## add ## mult ## xyname

    #define GB_AxB_WORKER(add,mult,xyname)          \
    {                                               \
        info = GB_Adot4B (add,mult,xyname) (C,      \
            A, A_is_pattern, A_slice, naslice,      \
            B, B_is_pattern, B_slice, nbslice,      \
            nthreads) ;                             \
        done = (info != GrB_NO_VALUE) ;             \
    }                                               \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xycode, zcode ;

    if (GB_AxB_semiring_builtin (A, A_is_pattern, B, B_is_pattern, semiring,
        flipxy, &mult_opcode, &add_opcode, &xycode, &zcode))
    { 
        #include "GB_AxB_factory.c"
    }

#endif

    //--------------------------------------------------------------------------
    // C += A'*B, computing each entry with a dot product, with typecasting
    //--------------------------------------------------------------------------

    if (!done)
    {
        GB_BURBLE_MATRIX (C, "generic ") ;

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A, B, C, and M
        //----------------------------------------------------------------------

        GxB_binary_function fmult = mult->function ;
        GxB_binary_function fadd  = add->op->function ;

        size_t csize = C->type->size ;
        size_t asize = A_is_pattern ? 0 : A->type->size ;
        size_t bsize = B_is_pattern ? 0 : B->type->size ;

        size_t xsize = mult->xtype->size ;
        size_t ysize = mult->ytype->size ;

        // scalar workspace: because of typecasting, the x/y types need not
        // be the same as the size of the A and B types.
        // flipxy false: aki = (xtype) A(k,i) and bkj = (ytype) B(k,j)
        // flipxy true:  aki = (ytype) A(k,i) and bkj = (xtype) B(k,j)
        size_t aki_size = flipxy ? ysize : xsize ;
        size_t bkj_size = flipxy ? xsize : ysize ;

        GB_void *GB_RESTRICT terminal = add->terminal ;

        GB_cast_function cast_A, cast_B ;
        if (flipxy)
        { 
            // A is typecasted to y, and B is typecasted to x
            cast_A = A_is_pattern ? NULL : 
                     GB_cast_factory (mult->ytype->code, A->type->code) ;
            cast_B = B_is_pattern ? NULL : 
                     GB_cast_factory (mult->xtype->code, B->type->code) ;
        }
        else
        { 
            // A is typecasted to x, and B is typecasted to y
            cast_A = A_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, A->type->code) ;
            cast_B = B_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = A'*B via dot products, function pointers, and typecasting
        //----------------------------------------------------------------------

        // aki = A(k,i), located in Ax [pA]
        #define GB_GETA(aki,Ax,pA)                                          \
            GB_void aki [GB_VLA(aki_size)] ;                                \
            if (!A_is_pattern) cast_A (aki, Ax +((pA)*asize), asize)

        // bkj = B(k,j), located in Bx [pB]
        #define GB_GETB(bkj,Bx,pB)                                          \
            GB_void bkj [GB_VLA(bkj_size)] ;                                \
            if (!B_is_pattern) cast_B (bkj, Bx +((pB)*bsize), bsize)

        // break if cij reaches the terminal value
        #define GB_DOT_TERMINAL(cij)                                        \
            if (terminal != NULL && memcmp (cij, terminal, csize) == 0)     \
            {                                                               \
                break ;                                                     \
            }

        // C(i,j) += A(i,k) * B(k,j)
        #define GB_MULTADD(cij, aki, bkj)                                   \
            GB_void zwork [GB_VLA(csize)] ;                                 \
            GB_MULTIPLY (zwork, aki, bkj) ;                                 \
            fadd (cij, cij, zwork)

        // define cij for each task
        #define GB_CIJ_DECLARE(cij)                                         \
            GB_void cij [GB_VLA(csize)]

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        // cij = Cx [p]
        #define GB_GETC(cij,pC)                                             \
            memcpy (cij, GB_CX (pC), csize)

        // Cx [p] = cij
        #define GB_PUTC(cij,pC)                                             \
            memcpy (GB_CX (pC), cij, csize)

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_VECTORIZE
        #define GB_PRAGMA_VECTORIZE_DOT

        if (flipxy)
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,y,x)
            #include "GB_AxB_dot4_template.c"
            #undef GB_MULTIPLY
        }
        else
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,x,y)
            #include "GB_AxB_dot4_template.c"
            #undef GB_MULTIPLY
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "dot: C += A'*B output", GB0) ;
    return (GrB_SUCCESS) ;
}

