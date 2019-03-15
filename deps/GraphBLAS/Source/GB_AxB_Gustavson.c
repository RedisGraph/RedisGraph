//------------------------------------------------------------------------------
// GB_AxB_Gustavson: C=A*B or C<M>=A*B, gather/scatter-based saxpy method.
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This method is agnostic to the CSR/CSC format.  The format of C is set
// to CSC but this is a placeholder that will be changed in GB_AxB_meta.

// parallel: this work is done by a single thread.  parallelism will be done in
// GB_AxB_parallel.

// Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_AxB__semirings.h"
#endif

// C=A*B failed, free everything, even the Sauna
#define GB_FREE_ALL                         \
{                                           \
    GB_MATRIX_FREE (Chandle) ;              \
    GB_Sauna_free (Sauna_id) ;              \
}

GrB_Info GB_AxB_Gustavson           // C=A*B or C<M>=A*B, Gustavson's method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // optional matrix
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_Context Context = NULL ;
    ASSERT (Chandle != NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M_in, "M for Gustavson C<M>=A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for Gustavson C=A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for Gustavson C=A*B", GB0)) ;
    ASSERT (!GB_PENDING (M_in)) ; ASSERT (!GB_ZOMBIES (M_in)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (A->vdim == B->vlen) ;
    ASSERT_OK (GB_check (semiring, "semiring for Gustavson A*B", GB0)) ;
    ASSERT (Sauna_id >= 0 && Sauna_id < GxB_NTHREADS_MAX) ;
    ASSERT (mask_applied != NULL) ;

    //--------------------------------------------------------------------------
    // determine size and hypersparsity of C
    //--------------------------------------------------------------------------

    GrB_Info info ;

    (*Chandle) = NULL ;

    // Gustavson's method does not handle a complemented mask
    GrB_Matrix M = (Mask_comp ? NULL : M_in) ;

    int64_t cvlen = A->vlen ;
    int64_t cvdim = B->vdim ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;

    // flipxy: if true, then compute z = fmult (B(k,j), A(i,k)) instead of the
    // usual z = fmult (A(i,k), B(k,j)), since A and B have been swapped on
    // input.

    // these conditions have already been checked in the caller
    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->ytype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->xtype)) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->xtype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->ytype)) ;
    }

    // these asserts hold for any valid semiring:
    ASSERT (mult->ztype == add->op->ztype) ;
    ASSERT (add->op->ztype == add->op->xtype) ;
    ASSERT (add->op->ztype == add->op->ytype) ;

    size_t zsize = mult->ztype->size ;

    //--------------------------------------------------------------------------
    // allocate the Sauna
    //--------------------------------------------------------------------------

    GB_Sauna Sauna = GB_Global.Saunas [Sauna_id] ;

    if (Sauna == NULL || Sauna->Sauna_n < cvlen || Sauna->Sauna_size < zsize)
    { 
        // get a new Sauna: the Sauna either does not exist, or is too small
        GB_Sauna_free (Sauna_id) ;
        GB_OK (GB_Sauna_alloc (Sauna_id, cvlen, zsize)) ;
        Sauna = GB_Global.Saunas [Sauna_id] ;
    }

    int64_t *restrict Sauna_Mark = Sauna->Sauna_Mark ;

    // Sauna_Mark [0..cvlen-1] < hiwater holds
    ASSERT_SAUNA_IS_RESET ;

    //--------------------------------------------------------------------------
    // estimate nnz(C) and allocate C (just the pattern)
    //--------------------------------------------------------------------------

    GB_OK (GB_AxB_alloc (Chandle, GrB_BOOL, cvlen, cvdim, M, A, B, false,
        cvlen + GB_NNZ (A) + GB_NNZ (B))) ;

    GrB_Matrix C = (*Chandle) ;
    ASSERT (C != NULL) ;
    ASSERT (C->x == NULL) ;

    //==========================================================================
    // symbolic analysis when no mask is present
    //==========================================================================

    if (M == NULL)
    {
        bool A_is_hyper = GB_IS_HYPER (A) ;
        if (A_is_hyper || GB_IS_HYPER (B) || GB_IS_HYPER (C))
        { 
            // symbolic analysis when one or more matrix is hypersparse
            #define GB_HYPER_CASE
            #include "GB_AxB_Gustavson_symbolic.c"
            #undef GB_HYPER_CASE
        }
        else
        { 
            // symbolic analysis when no matrix is hypersparse
            #include "GB_AxB_Gustavson_symbolic.c"
        }

        // FUTURE: if A and B are pattern-only and the semiring is AND_OR
        // or OR_AND (perhaps others) the C is pattern-only, and the values
        // of C do not need to be computed.  The work is done here.
    }

    //==========================================================================
    // numerical phase
    //==========================================================================

    //--------------------------------------------------------------------------
    // allocate C->x
    //--------------------------------------------------------------------------

    // C has the same type as z for z=fmult(x,y).  The type is also the same as
    // the monoid of the semiring.  The types of A and/or B, and their
    // typecasting, has no effect on the size of the type of C or the Sauna
    // workspace.

    C->type = mult->ztype ;
    C->type_size = zsize ;

    char zwork [zsize] ;

    GB_MALLOC_MEMORY (C->x, C->nzmax, zsize) ;
    if (C->x == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    C->x_shallow = false ;

    // Sauna_Work has size cvlen, each entry of size zsize.  Not initialized.
    GB_void *restrict Sauna_Work = Sauna->Sauna_Work ;

    if (M != NULL)
    { 
        ASSERT (M->vlen == C->vlen && M->vdim == C->vdim) ;
    }

    //--------------------------------------------------------------------------
    // determine the required types of A and B, for typecasting
    //--------------------------------------------------------------------------

    GrB_Type atype_required, btype_required ;

    if (flipxy)
    { 
        // A is passed as y, and B as x, in z = mult(x,y)
        atype_required = mult->ytype ;
        btype_required = mult->xtype ;
    }
    else
    { 
        // A is passed as x, and B as y, in z = mult(x,y)
        atype_required = mult->xtype ;
        btype_required = mult->ytype ;
    }

    bool no_typecasting = (A->type == atype_required)
                       && (B->type == btype_required) ;

    //--------------------------------------------------------------------------
    // compute C = A*B for built-in types and operators
    //--------------------------------------------------------------------------

    if (no_typecasting)
    { 

        ASSERT_OK (GB_check (A->type, "A type for Gustavson builtin", GB0)) ;
        ASSERT_OK (GB_check (B->type, "B type for Gustavson builtin", GB0)) ;
        ASSERT_OK (GB_check (C->type, "C type for Gustavson builtin", GB0)) ;

#ifndef GBCOMPACT

        // If the GB_AxB_Gustavson_builtin function has a worker for the
        // particular semiring, then it does the computation and returns done =
        // true.  Otherwise, it returns done as false, and the generic worker
        // below does the work.

        // If GBCOMPACT is enabled at compile-time, then no built-in workers
        // are created, and this function is not used.  All C=A*B computations
        // are done with the generic worker below.

        bool done = false ;
        info = GB_AxB_Gustavson_builtin (C, M, A, B, semiring, flipxy, &done,
            Sauna) ;
        ASSERT (info == GrB_SUCCESS) ;
        if (done)
        { 
            // C = A*B has been done via a hard-coded case
            ASSERT_OK (GB_check (C, "C hard-coded for Gustavson C=A*B", GB0)) ;
            ASSERT (*Chandle == C) ;
            ASSERT_SAUNA_IS_RESET ;
            (*mask_applied) = (M != NULL) ;
            return (GrB_SUCCESS) ;
        }

#endif

        //----------------------------------------------------------------------
        // user semirings created at compile time
        //----------------------------------------------------------------------

        // A precompiled function for C=A*B with user-defined semiring is
        // called only if no typecasting is needed.

        if (semiring->object_kind == GB_USER_COMPILED)
        { 
            info = GB_AxB_user (GxB_AxB_GUSTAVSON, semiring, Chandle, M, A, B,
                flipxy, false, NULL, NULL, NULL, 0, Sauna) ;
            (*mask_applied) = (M != NULL) && (info == GrB_SUCCESS) ;
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // generic Gustavson, any semiring, with or without typecasting
    //--------------------------------------------------------------------------

    // Define operations for GB_AxB_Gustavson_mask and GB_AxB_Gustavson_nomask,
    // whether or not typecasting is needed.

    #define GB_IDENTITY \
        identity

    // x [i] = y        // to scatter C(i,j) to Work(i), of size zsize
    #define GB_COPY_SCALAR_TO_ARRAY(x,i,y,s)        \
        memcpy (x +((i)*s), y, s) ;

    // x [i] = y [j]    // to gather C(i,j) from Work(i), of size zsize
    #define GB_COPY_ARRAY_TO_ARRAY(x,i,y,j,s)       \
        memcpy (x +((i)*s), y +((j)*s), s) ;

    // z = x*y
    #define GB_MULTOP(z,x,y) fmult (z, x, y) ;

    size_t asize = A->type->size ;
    size_t bsize = B->type->size ;

    size_t xsize = mult->xtype->size ;
    size_t ysize = mult->ytype->size ;

    // scalar workspace (after typecasting, if needed)
    // flipxy false: aik = (xtype) A(i,k) and bkj = (ytype) B(k,j)
    // flipxy true:  aik = (ytype) A(i,k) and bkj = (xtype) B(k,j)
    char aik [flipxy ? ysize : xsize] ;
    char bkj [flipxy ? xsize : ysize] ;

    GxB_binary_function fmult = mult->function ;
    GxB_binary_function fadd  = add->op->function ;
    GB_void *restrict identity = add->identity ;
    GB_void *restrict Cx = C->x ;
    #define GB_HANDLE_FLIPXY true
    #define GB_XTYPE GB_void
    #define GB_YTYPE GB_void

    if (no_typecasting)
    { 

        //----------------------------------------------------------------------
        // generic C=A*B or C<M>=A*B with any semiring, but no typecasting
        //----------------------------------------------------------------------

        // x = y [i]        // for bkj = B(k,j), of size bsize
        #define GB_COPY_ARRAY_TO_SCALAR(x,y,i,s)                            \
            memcpy (x, y +((i)*s), s) ;

        // generic multiply-add operation (with no mask).  fadd: (z x alias)
        #define GB_MULTADD_NOMASK                                           \
        {                                                                   \
            /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
            GB_MULTIPLY (zwork, Ax +(pA*asize), bkj) ;                      \
            fadd (Sauna_Work +(i*zsize), Sauna_Work +(i*zsize), zwork) ;    \
        }

        // generic multiply-add operation (with mask)
        #define GB_MULTADD_WITH_MASK                                        \
        {                                                                   \
            /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
            if (mark == hiwater)                                            \
            {                                                               \
                /* first time C(i,j) seen */                                \
                /* Sauna_Work [i] = A(i,k) * B(k,j) */                      \
                GB_MULTIPLY (Sauna_Work +(i*zsize), Ax +(pA*asize), bkj) ;  \
                Sauna_Mark [i] = hiwater + 1 ;                              \
            }                                                               \
            else                                                            \
            {                                                               \
                /* C(i,j) seen before, update it */                         \
                /* Sauna_Work [i] += A(i,k) * B(k,j) */                     \
                GB_MULTADD_NOMASK ;                                         \
            }                                                               \
        }

        #include "GB_AxB_Gustavson_flipxy.c"

    }
    else
    { 

        //----------------------------------------------------------------------
        // generic C=A*B or C<M>=A*B with any semiring, with any typecasting
        //----------------------------------------------------------------------

        GB_cast_function cast_A, cast_B ;
        if (flipxy)
        { 
            // A is typecasted to y, and B is typecasted to x
            cast_A = GB_cast_factory (mult->ytype->code, A->type->code) ;
            cast_B = GB_cast_factory (mult->xtype->code, B->type->code) ;
        }
        else
        { 
            // A is typecasted to x, and B is typecasted to y
            cast_A = GB_cast_factory (mult->xtype->code, A->type->code) ;
            cast_B = GB_cast_factory (mult->ytype->code, B->type->code) ;
        }

        // x = y [i]        // for bkj = B(k,j), of size bsize
        #undef  GB_COPY_ARRAY_TO_SCALAR
        #define GB_COPY_ARRAY_TO_SCALAR(x,y,i,s)                            \
            cast_B (x, y +((i)*s), s) ;

        // generic multiply-add operation (with no mask).  fadd: (z x alias)
        #undef  GB_MULTADD_NOMASK
        #define GB_MULTADD_NOMASK                                           \
        {                                                                   \
            /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
            cast_A (aik, Ax +(pA*asize), asize) ;                           \
            GB_MULTIPLY (zwork, aik, bkj) ;                                 \
            fadd (Sauna_Work +(i*zsize), Sauna_Work +(i*zsize), zwork) ;    \
        }

        // generic multiply-add operation (with mask)
        #undef  GB_MULTADD_WITH_MASK
        #define GB_MULTADD_WITH_MASK                                        \
        {                                                                   \
            /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
            if (mark == hiwater)                                            \
            {                                                               \
                /* first time C(i,j) seen */                                \
                /* Sauna_Work [i] = A(i,k) * B(k,j) */                      \
                cast_A (aik, Ax +(pA*asize), asize) ;                       \
                GB_MULTIPLY (Sauna_Work +(i*zsize), aik, bkj) ;             \
                Sauna_Mark [i] = hiwater + 1 ;                              \
            }                                                               \
            else                                                            \
            {                                                               \
                /* C(i,j) seen before, update it */                         \
                /* Sauna_Work [i] += A(i,k) * B(k,j) */                     \
                GB_MULTADD_NOMASK ;                                         \
            }                                                               \
        }

        #include "GB_AxB_Gustavson_flipxy.c"

    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_SAUNA_IS_RESET ;

    // cannot fail since C->plen is the upper bound: # non-empty columns of B
    ASSERT (info == GrB_SUCCESS) ;
    // if it could fail, do this:
    // GB_OK (info) ;     // check result and return if an error occurred

    ASSERT_OK (GB_check (C, "C output for Gustavson C=A*B", GB0)) ;
    ASSERT (*Chandle == C) ;
    (*mask_applied) = (M != NULL) ;
    return (GrB_SUCCESS) ;
}

